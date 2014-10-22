#include <iostream>
#include "opencv2/core/core.hpp"

class OpticalFlow {

public:
  OpticalFlow(double pyrScale = 0.5, int numLevels = 3, int winSize = 30, int numIters = 3, int polyN = 7, double polySigma = 1.5, int flags = cv::OPTFLOW_FARNEBACK_GAUSSIAN);

  virtual ~OpticalFlow() {};

  float calculate(std::string expectImgPath, std::string targetImgPath, bool gpuMode, cv::Mat &flowx, cv::Mat &flowy);

protected:
  virtual float calculateInternal(cv::Mat expectImg, cv::Mat targetImg, cv::Mat &flowx, cv::Mat &flowy) = 0;

  double pyrScale;
  int numLevels;
  int winSize;
  int numIters;
  int polyN;
  double polySigma;
  int flags;
};

class OpticalFlowByCPU : public OpticalFlow {
public:
  virtual float calculateInternal(cv::Mat expectImg, cv::Mat targetImg, cv::Mat &flowx, cv::Mat &flowy);
};

#ifdef USE_GPU
class OpticalFlowByGPU : public OpticalFlow {
public:
  virtual float calculateInternal(cv::Mat expectImg, cv::Mat targetImg, cv::Mat &flowx, cv::Mat &flowy);
};
#endif
