#include <iostream>
#include <vector>
#include <sstream>

#include <node.h>
#include <v8.h>

#include "opencv2/core/core.hpp"

class OpticalFlow : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object>& target);
  void Emit(
    const std::string expect_image, const std::string target_image,
    const cv::Mat *flowx, const cv::Mat *flowy,
    const double threshold, const int span,
    const float time
  );
  void Finish();
private:
  OpticalFlow();
  ~OpticalFlow();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);

  static v8::Handle<v8::Value> Calc(const v8::Arguments& args);

};
