#include <iostream>
#include <vector>
#include <sstream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/gpu/gpu.hpp"

#include <node.h>
#include <v8.h>

using namespace v8;
using namespace std;


int consprod(const string expect_path, const string target_path, const double threshold, const int span, Local<Function> &cb);

Handle<Value> Method(const Arguments& args) {
  HandleScope scope;

  
  v8::String::Utf8Value param1(args[0]->ToString());
  v8::String::Utf8Value param2(args[1]->ToString());

  string image1 = std::string(*param1);
  string image2 = std::string(*param2);

  double threshold = args[2]->NumberValue();
  int span = args[3]->IntegerValue();

  Local<Function> cb = Local<Function>::Cast(args[4]);
  consprod(image1, image2, threshold, span, cb);

  return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("opticalflow"),
      FunctionTemplate::New(Method)->GetFunction());
}

NODE_MODULE(opticalflow, init)
