#include <iostream>
#include <vector>
#include <sstream>

#include <node.h>
#include <v8.h>

#include "opencv2/core/core.hpp"

#include "main.h"

using namespace v8;
using namespace node;
using namespace std;

static Persistent<String> emit_symbol;


int dispatch(
  const string expect_path,
  const string target_path,
  const double threshold,
  const int span, 
  OpticalFlow *opt
);

// 解析結果をNode.jsで扱える型で生成
static void createResult(
  const string expect_image, const string target_image,
  const cv::Mat *flowx, const cv::Mat *flowy,
  const double threshold, const int span,
  const float time,
  const Local<Object> &result) {

  Persistent<String> time_symbol = NODE_PSYMBOL("time");
  Persistent<String> status_symbol = NODE_PSYMBOL("status");

  Persistent<String> span_symbol = NODE_PSYMBOL("span");
  Persistent<String> threshold_symbol = NODE_PSYMBOL("threshold");
  Persistent<String> expect_symbol = NODE_PSYMBOL("expect_image");
  Persistent<String> target_symbol = NODE_PSYMBOL("target_image");

  Persistent<String> vector_symbol = NODE_PSYMBOL("vector");
  Persistent<String> x_symbol = NODE_PSYMBOL("x");
  Persistent<String> y_symbol = NODE_PSYMBOL("y");
  Persistent<String> dx_symbol = NODE_PSYMBOL("dx");
  Persistent<String> dy_symbol = NODE_PSYMBOL("dy");

  result->Set(span_symbol, Integer::New(span));
  result->Set(threshold_symbol, Number::New(threshold));
  result->Set(expect_symbol, String::New(expect_image.c_str()));
  result->Set(target_symbol, String::New(target_image.c_str()));

  result->Set(time_symbol, Number::New(time));


  Local<Array> vectors = Array::New();
  int vector_len = 0;
  for (int y = 0; y < flowx->rows; ++y) {
    if ( y % span != 0 ) continue;
    for (int x = 0; x < flowx->cols; ++x) {
      if ( x % span != 0 ) continue;
      float dx = flowx->at<float>(y,x);
      float dy = flowy->at<float>(y,x);
      float len = (dx*dx) + (dy*dy);
      if (len > (threshold*threshold)) {
        Local<Object> v = Object::New();
        v->Set(x_symbol, Integer::New(x));
        v->Set(y_symbol, Integer::New(y));
        v->Set(dx_symbol, Number::New(dx));
        v->Set(dy_symbol, Number::New(dy));
        vectors->Set(vector_len++, v);
      }
    }
  }
  result->Set(vector_symbol, vectors);
}

void OpticalFlow::Init(Handle<Object>& target) {
    Local<FunctionTemplate> clazz = FunctionTemplate::New(OpticalFlow::New);
    clazz->SetClassName(String::NewSymbol("OpticalFlow"));
    clazz->InstanceTemplate()->SetInternalFieldCount(1);
    clazz->PrototypeTemplate()->Set(
      String::NewSymbol("calc"),
      FunctionTemplate::New(OpticalFlow::Calc)->GetFunction()
    );
    target->Set(String::NewSymbol("OpticalFlow"), clazz->GetFunction());
  };

OpticalFlow::OpticalFlow()
    : ObjectWrap() {};

OpticalFlow::~OpticalFlow() {};

Handle<Value> OpticalFlow::New(const Arguments& args) {
    HandleScope scope;
    OpticalFlow *opt = new OpticalFlow();
    opt->Wrap(args.This());
    return args.This();
  };

Handle<Value> OpticalFlow::Calc(const Arguments& args) {
    HandleScope scope;

    if (args.Length() < 4) {
      ThrowException(Exception::TypeError(String::New("5 arguments expected")));
      return scope.Close(Undefined());
    }

    if (!args[0]->IsString()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments(expect_path)")));
      return scope.Close(Undefined());
    }
    if (!args[1]->IsString()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments(target_path)")));
      return scope.Close(Undefined());
    }
    if (!args[2]->IsNumber()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments(threshold)")));
      return scope.Close(Undefined());
    }
    if (!args[3]->IsNumber()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments(span)")));
      return scope.Close(Undefined());
    }

    v8::String::Utf8Value param1(args[0]->ToString());
    v8::String::Utf8Value param2(args[1]->ToString());

    string expect_path = string(*param1);
    string target_path = string(*param2);

    double threshold = args[2]->NumberValue();
    int span = args[3]->IntegerValue();

    OpticalFlow *opt = ObjectWrap::Unwrap<OpticalFlow>(args.This());
    dispatch(expect_path, target_path, threshold, span, opt);

    return scope.Close(Undefined());
  };

void OpticalFlow::Emit(
    const string expect_image, const string target_image,
    const cv::Mat *flowx, const cv::Mat *flowy,
    const double threshold, const int span,
    const float time
  ) {
    HandleScope scope;
    Local<Value> emit_v = handle_->Get(emit_symbol);
    Local<Function> emit = emit_v.As<Function>();

    Local<Object> result = Object::New();
    createResult(
      expect_image, target_image,
      flowx, flowy,
      threshold, span,
      time,
      result);

    // 結果をコールバック
    const unsigned argc = 2;
    Local<Value> argv[argc] = { 
      String::New("message"),
      Local<Value>::New(result) 
    };
    emit->Call(this->handle_, argc, argv);
  };

void init(Handle<Object> target) {
  emit_symbol = NODE_PSYMBOL("emit");
  OpticalFlow::Init(target);
}

NODE_MODULE(opticalflow, init)
