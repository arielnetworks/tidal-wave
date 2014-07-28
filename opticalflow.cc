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


float calc_opticalflow(string pathL, string pathR, bool gpuMode, cv::Mat &flowx, cv::Mat &flowy);

static void copyToNode(const cv::Mat &flowx, const cv::Mat &flowy, const Local<Array> &results) {
    Persistent<String> x_symbol = NODE_PSYMBOL("x");
    Persistent<String> y_symbol = NODE_PSYMBOL("y");
    Persistent<String> len_symbol = NODE_PSYMBOL("len");

    int i = 0;
    for (int y = 0; y < flowx.rows; ++y) {
        if ( y % 5 != 0 ) continue;
        for (int x = 0; x < flowx.cols; ++x) {
            if ( x % 5 != 0 ) continue;
            float dx = flowx.at<float>(y,x);
            float dy = flowy.at<float>(y,x);
            float len = (dx*dx) + (dy*dy);
            if (len > 25) {
                Local<Object> obj = Object::New();
                obj->Set(x_symbol, Integer::New(x));
                obj->Set(y_symbol, Integer::New(y));
                obj->Set(len_symbol, Number::New(len));
                results->Set(i++, obj);
            }
        }
    }
}

Handle<Value> Method(const Arguments& args) {
  HandleScope scope;
  
  v8::String::Utf8Value param1(args[0]->ToString());
  v8::String::Utf8Value param2(args[1]->ToString());

  string image1 = std::string(*param1);
  string image2 = std::string(*param2);

  Local<Function> cb = Local<Function>::Cast(args[2]);
  cv::Mat flowx, flowy;
  float t = calc_opticalflow(image1, image2, false, flowx, flowy);

  Local<Array> results = Array::New();
  copyToNode(flowx, flowy, results);
  
  const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(results) };
  cb->Call(Context::GetCurrent()->Global(), argc, argv);
  return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("opticalflow"),
      FunctionTemplate::New(Method)->GetFunction());
}

NODE_MODULE(opticalflow, init)
