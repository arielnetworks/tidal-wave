#ifndef OPTICAL_FLOW_H
#define OPTICAL_FLOW_H

#include <iostream>
#include "opencv2/core/core.hpp"

namespace tidalwave {

  enum ErrorCode {
    OK,
    BadParameter,
    BadImageFormat,
    DontMatchSize
  };

  /*
   * @brief OpticalFlowの解析結果のステータス
   * Node-gypでは例外が使えないので、代わりにこんなデータ構造を使う
   */
  struct OpticalFlowStatus {
    ErrorCode code;
    std::string message;
    float time;
  };

  struct OpticalFlowParameter{
    double pyrScale;
    int pyrLevels;
    int winSize;
    int pyrIterations;
    int polyN;
    double polySigma;
    int flags;
  };

  class OpticalFlow {

  public:
    OpticalFlow();

    virtual ~OpticalFlow() {
    };

    OpticalFlowStatus calculate(const std::string &expectImgPath, const std::string &targetImgPath, const OpticalFlowParameter &param, cv::Mat &flowx, cv::Mat &flowy);

  protected:
    virtual float calculateInternal(cv::Mat &expectImg, cv::Mat &targetImg, cv::Mat &flowx, cv::Mat &flowy) = 0;
    OpticalFlowParameter parameter;

  };

  class OpticalFlowByCPU : public OpticalFlow {
  public:
    OpticalFlowByCPU() : OpticalFlow() {
    };

    virtual float calculateInternal(cv::Mat &expectImg, cv::Mat &targetImg, cv::Mat &flowx, cv::Mat &flowy);
  };

#ifdef USE_GPU

  class OpticalFlowByGPU : public OpticalFlow {
  public:
    OpticalFlowByGPU() : OpticalFlow() {
    };

    virtual float calculateInternal(cv::Mat &expectImg, cv::Mat &targetImg, cv::Mat &flowx, cv::Mat &flowy);
  };

#endif
}

#endif // OPTICAL_FLOW_H

