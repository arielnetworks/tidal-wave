#include <iostream>
#include <vector>
#include <sstream>

#include <node.h>
#include <v8.h>

using namespace v8;
using namespace std;

int dispatch(const string expect_path, const string target_path, const double threshold, const int span, Local<Function> &cb);

Handle<Value> Method(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 5) {
    ThrowException(Exception::TypeError(String::New("5 arguments expected")));
    return scope.Close(Undefined());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }
  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }
  if (!args[2]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }
  if (!args[3]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }
  if (!args[4]->IsFunction()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }

  v8::String::Utf8Value param1(args[0]->ToString());
  v8::String::Utf8Value param2(args[1]->ToString());

  string expect_path = string(*param1);
  string target_path = string(*param2);

  double threshold = args[2]->NumberValue();
  int span = args[3]->IntegerValue();

  Local<Function> cb = Local<Function>::Cast(args[4]);
  dispatch(expect_path, target_path, threshold, span, cb);

  return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("opticalflow"),
      FunctionTemplate::New(Method)->GetFunction());
}

NODE_MODULE(opticalflow, init)
