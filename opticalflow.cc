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


int consprod();
float calc_opticalflow(string pathL, string pathR, bool gpuMode, cv::Mat &flowx, cv::Mat &flowy);

static void copyToNode(const cv::Mat &flowx, const cv::Mat &flowy, const double threshold, const int span, const Local<Array> &results) {
    Persistent<String> x_symbol = NODE_PSYMBOL("x");
    Persistent<String> y_symbol = NODE_PSYMBOL("y");
    Persistent<String> dx_symbol = NODE_PSYMBOL("dx");
    Persistent<String> dy_symbol = NODE_PSYMBOL("dy");
    Persistent<String> len_symbol = NODE_PSYMBOL("len");

    int i = 0;
    for (int y = 0; y < flowx.rows; ++y) {
        if ( y % span != 0 ) continue;
        for (int x = 0; x < flowx.cols; ++x) {
            if ( x % span != 0 ) continue;
            float dx = flowx.at<float>(y,x);
            float dy = flowy.at<float>(y,x);
            float len = (dx*dx) + (dy*dy);
            if (len > (threshold*threshold)) {
                Local<Object> obj = Object::New();
                obj->Set(x_symbol, Integer::New(x));
                obj->Set(y_symbol, Integer::New(y));
                obj->Set(dx_symbol, Number::New(dx));
                obj->Set(dy_symbol, Number::New(dy));
                //obj->Set(len_symbol, Number::New(len));
                results->Set(i++, obj);
            }
        }
    }
}

Handle<Value> Method(const Arguments& args) {
  HandleScope scope;

  consprod();
  
  v8::String::Utf8Value param1(args[0]->ToString());
  v8::String::Utf8Value param2(args[1]->ToString());

  string image1 = std::string(*param1);
  string image2 = std::string(*param2);

  double threshold = args[2]->NumberValue();
  int span = args[3]->IntegerValue();

  Local<Function> cb = Local<Function>::Cast(args[4]);
  cv::Mat flowx, flowy;
  float t = calc_opticalflow(image1, image2, false, flowx, flowy);

  Local<Array> results = Array::New();
  copyToNode(flowx, flowy, threshold, span, results);
  
  const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(results) };
  cb->Call(Context::GetCurrent()->Global(), argc, argv);
  std::cout << "time: " << t << std::endl;
  return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("opticalflow"),
      FunctionTemplate::New(Method)->GetFunction());
}

NODE_MODULE(opticalflow, init)
