#include <vector>
#include <sstream>

#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opticalflow.h"

using namespace std;
using namespace cv;

#ifdef USE_GPU
#include "opencv2/gpu/gpu.hpp"
using namespace cv::gpu;
#endif

namespace tidalwave {
  OpticalFlow::OpticalFlow() {
  }

  OpticalFlowStatus OpticalFlow::calculate(const string &expectImgPath, const string &targetImgPath, const OpticalFlowParameter &param, Mat &flowx, Mat &flowy) {
    OpticalFlowStatus status;
    status.code = OK;

    this->parameter = param;

    if (expectImgPath.empty()) {
      status.code = BadParameter;
      status.message = "ExpectImagePath is empty.";
      return status;
    }
    if (targetImgPath.empty()) {
      status.code = BadParameter;
      status.message = "TargetImagePath is empty.";
      return status;
    }

    Mat expectImg = imread(expectImgPath, IMREAD_GRAYSCALE);
    if (expectImg.empty()) {
      status.code = BadImageFormat;
      status.message = "Can't open " + expectImgPath;
      return status;
    }

    Mat targetImg = imread(targetImgPath, IMREAD_GRAYSCALE);
    if (targetImg.empty()) {
      status.code = BadImageFormat;
      status.message = "Can't open " + targetImgPath;
      return status;
    }

    // 比較する画像のサイズが違い過ぎたらエラー
    if (abs(expectImg.rows - targetImg.rows) > 5 || abs(expectImg.cols - targetImg.cols) > 5) {
      status.code = DontMatchSize;
      status.message = "Don't match image size";
      /*
      status.message = string("Don't match image size: (")
        + expectImg.rows + ", " + expectImg.cols + ") "
        + " (" + targetImg.rows + ", " + targetImg.cols + ") ";
      */
      return status;
    }

    // 画像のサイズが多少違うだけだったら、リサイズする
    if (expectImg.rows != targetImg.rows || expectImg.cols != targetImg.cols) {
      Mat resizedTargetImg(expectImg.rows, expectImg.cols, expectImg.type());
      cv::resize(targetImg, resizedTargetImg, resizedTargetImg.size(), cv::INTER_NEAREST);
      targetImg = resizedTargetImg;
    }

    status.time = calculateInternal(expectImg, targetImg, flowx, flowy);
    return status;
  }

  float OpticalFlowByCPU::calculateInternal(Mat &expectImg, Mat &targetImg, Mat &flowx, Mat &flowy) {
    int64 tc0, tc1;
    Mat flowxy;

    tc0 = cv::getTickCount();
    cv::calcOpticalFlowFarneback(
        expectImg, targetImg, flowxy, parameter.pyrScale, parameter.pyrLevels, parameter.winSize,
        parameter.pyrIterations, parameter.polyN, parameter.polySigma, parameter.flags);
    tc1 = cv::getTickCount();

    Mat planes[] = {flowx, flowy};
    split(flowxy, planes);
    flowx = planes[0];
    flowy = planes[1];

    return (tc1 - tc0) / getTickFrequency();
  }

#ifdef USE_GPU
  float OpticalFlowByGPU::calculateInternal(Mat &expectImg, Mat &targetImg, Mat &flowx, Mat &flowy) {
    int64 tc0, tc1;

    GpuMat d_frameL(expectImg), d_targetImg(targetImg);
    GpuMat d_flowx, d_flowy;
    FarnebackOpticalFlow d_calc;

    d_calc.pyrScale = parameter.pyrScale;
    d_calc.pyrLevels = parameter.pyrLevels;
    d_calc.winSize = parameter.winSize;
    d_calc.pyrIterations = parameter.pyrIterations;
    d_calc.polyN = parameter.polyN;
    d_calc.polySigma = parameter.polySigma;
    d_calc.flags = parameter.flags;

    tc0 = cv::getTickCount();
    d_calc(d_frameL, d_targetImg, d_flowx, d_flowy);
    tc1 = cv::getTickCount();
    d_flowx.download(flowx);
    d_flowy.download(flowy);

    return (tc1 - tc0) / getTickFrequency();
  }

#endif
}
