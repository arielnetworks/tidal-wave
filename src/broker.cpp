#include <iostream>
#include <node.h>
#include "opencv2/core/core.hpp"
#include "broker.h"

using namespace v8;
using namespace node;
using namespace std;

namespace tidalwave {
  Manager *Broker::manager;
  Broker *Broker::broker;

  static Persistent<String> STATUS_SYMBOL = NODE_PSYMBOL("status");
  static Persistent<String> REASON_SYMBOL = NODE_PSYMBOL("reason");
  static Persistent<String> SPAN_SYMBOL = NODE_PSYMBOL("span");
  static Persistent<String> THRESHOLD_SYMBOL = NODE_PSYMBOL("threshold");
  static Persistent<String> EXPECT_IMAGE_SYMBOL = NODE_PSYMBOL("expect_image");
  static Persistent<String> TARGET_IMAGE_SYMBOL = NODE_PSYMBOL("target_image");
  static Persistent<String> TIME_SYMBOL = NODE_PSYMBOL("time");
  static Persistent<String> X_SYMBOL = NODE_PSYMBOL("x");
  static Persistent<String> Y_SYMBOL = NODE_PSYMBOL("y");
  static Persistent<String> DX_SYMBOL = NODE_PSYMBOL("dx");
  static Persistent<String> DY_SYMBOL = NODE_PSYMBOL("dy");
  static Persistent<String> VECTOR_SYMBOL = NODE_PSYMBOL("vector");
  static Persistent<String> TOTAL_SYMBOL = NODE_PSYMBOL("total");
  static Persistent<String> REPORTED_SYMBOL = NODE_PSYMBOL("reported");

  void Broker::initialize(Handle<Object> &target) {
    Local<FunctionTemplate> clazz = FunctionTemplate::New(Broker::createInstance);
    clazz->SetClassName(String::NewSymbol("OpticalFlow"));
    clazz->InstanceTemplate()->SetInternalFieldCount(1);
    clazz->PrototypeTemplate()->Set(
        String::NewSymbol("calc"),
        FunctionTemplate::New(Broker::requestFromClient)->GetFunction()
    );
    target->Set(String::NewSymbol("OpticalFlow"), clazz->GetFunction());
  };

  void Broker::onNext(const Response &value) {
    HandleScope scope;

    Local<Object> result = Broker::convertResult(value);
    // 結果をコールバック
    const unsigned argc = 2;
    Local<Value> argv[argc] = {
        String::New("message"),
        Local<Value>::New(result)
    };
    node::MakeCallback(this->handle_, "emit", argc, argv);
  };

  void Broker::onError(const string &err) {
    HandleScope scope;

    Local<Object> result = Object::New();
    result->Set(STATUS_SYMBOL, String::New("ERROR"));
    result->Set(REASON_SYMBOL, String::New(err.c_str()));

    const unsigned argc = 2;
    Local<Value> argv[argc] = {
        String::New("error"),
        Local<Value>::New(result)
    };
    node::MakeCallback(this->handle_, "emit", argc, argv);
  }

  void Broker::onCompleted() {
    HandleScope scope;

    const unsigned argc = 1;
    Local<Value> argv[argc] = {
        String::New("finish")
    };
    node::MakeCallback(this->handle_, "emit", argc, argv);
  };

  Broker::Broker()
      : ObjectWrap() {
  };

  Broker::~Broker() {
  };

  Handle<Value> Broker::createInstance(const Arguments &args) {
    HandleScope scope;
    Parameter param;

    param.threshold = getNumberOrDefault(args[2], "threshold", 5.0);
    param.span = getInt32OrDefault(args[2], "span", 10);

    param.numThreads = getInt32OrDefault(args[2], "numThreads", 4);

    param.optParam.pyrScale = getNumberOrDefault(args[2], "pyrScale", 0.5);
    param.optParam.pyrLevels = getInt32OrDefault(args[2], "pyrLevels", 3);
    param.optParam.winSize = getInt32OrDefault(args[2], "winSize", 30);
    param.optParam.pyrIterations = getInt32OrDefault(args[2], "pyrIterations", 3);
    param.optParam.polyN = getInt32OrDefault(args[2], "polyN", 7);
    param.optParam.polySigma = getNumberOrDefault(args[2], "polySigma", 1.5);
    param.optParam.flags = getInt32OrDefault(args[2], "flags", 256); // cv::OPTFLOW_FARNEBACK_GAUSSIAN

    Broker::broker = new Broker();
    Broker::manager = new Manager(Broker::broker);
    Broker::manager->start(param);

    broker->Wrap(args.This());
    return args.This();
  };

  Handle<Value> Broker::requestFromClient(const Arguments &args) {
    HandleScope scope;
    if (args.Length() != 2) {
      ThrowException(Exception::TypeError(String::New("2 arguments expected")));
      return scope.Close(Undefined());
    }

    if (!args[0]->IsString()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments(expect_image)")));
      return scope.Close(Undefined());
    }
    if (!args[1]->IsString()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments(target_image)")));
      return scope.Close(Undefined());
    }
    v8::String::Utf8Value param1(args[0]->ToString());
    v8::String::Utf8Value param2(args[1]->ToString());

    string expect_image = string(*param1);
    string target_image = string(*param2);

    Broker::manager->request(expect_image, target_image);

    return scope.Close(Undefined());
  };

  // 解析結果をNode.jsで扱える型で生成
  Local<Object> Broker::convertResult(const Response &value) {
    HandleScope scope;
    Local<Object> result = Object::New();

    result->Set(STATUS_SYMBOL, String::New(value.status.c_str()));
    result->Set(SPAN_SYMBOL, Integer::New(value.span));
    result->Set(THRESHOLD_SYMBOL, Number::New(value.threshold));
    result->Set(EXPECT_IMAGE_SYMBOL, String::New(value.expect_image.c_str()));
    result->Set(TARGET_IMAGE_SYMBOL, String::New(value.target_image.c_str()));
    result->Set(TIME_SYMBOL, Number::New(value.time));

    uint32_t vector_len = 0;
    Local<Array> vectors = Array::New();
    for (vector<Vector>::const_iterator it = value.vectors.begin(); it != value.vectors.end(); it++) {
      Vector v = *it;
      Local<Object> o = Object::New();
      o->Set(X_SYMBOL, Integer::New(v.x));
      o->Set(Y_SYMBOL, Integer::New(v.y));
      o->Set(DX_SYMBOL, Number::New(v.dx));
      o->Set(DY_SYMBOL, Number::New(v.dy));
      vectors->Set(vector_len++, o);
    }
    result->Set(VECTOR_SYMBOL, vectors);

    return scope.Close(result);
  }

  int Broker::getInt32OrDefault(const Local<Value> &arg, const string &name, int defaultValue) {
    if (!arg->IsUndefined() && arg->IsObject()) {
      Local<Object> options = arg->ToObject();
      if (!options->IsUndefined() && options->Has(String::NewSymbol(name.c_str())) && options->Get(String::NewSymbol(name.c_str()))->IsInt32()) {
        return options->Get(String::NewSymbol(name.c_str()))->Int32Value();
      }
    }
    return defaultValue;
  }

  double Broker::getNumberOrDefault(const Local<Value> &arg, const string &name, double defaultValue) {
    if (!arg->IsUndefined() && arg->IsObject()) {
      Local<Object> options = arg->ToObject();
      if (!options->IsUndefined() && options->Has(String::NewSymbol(name.c_str())) && options->Get(String::NewSymbol(name.c_str()))->IsNumber()) {
        return options->Get(String::NewSymbol(name.c_str()))->NumberValue();
      }
    }

    return defaultValue;
  }
}

