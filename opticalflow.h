#include <iostream>
#include "opencv2/core/core.hpp"

class OpticalFlow {
public:
  static float calculate(std::string expectImgPath, std::string targetImgPath, bool gpuMode, cv::Mat &flowx, cv::Mat &flowy);
private:
  ~OpticalFlow();
};

