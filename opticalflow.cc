#include <iostream>
#include <vector>
#include <sstream>

#include <node.h>
#include <v8.h>

using namespace v8;
using namespace std;

int calc_opticalflow(string pathL, string pathR, bool gpuMode);

Handle<Value> Method(const Arguments& args) {
  HandleScope scope;
  int t = calc_opticalflow("customjsp1.png", "customjsp2.png", false);
  return scope.Close(Integer::New(t));
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("hello"),
      FunctionTemplate::New(Method)->GetFunction());
}

NODE_MODULE(opticalflow, init)
