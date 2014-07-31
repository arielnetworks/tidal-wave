#include <iostream>
#include <vector>
#include <sstream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

float calcOpticalFlow(string expectImgPath, string targetImgPath, bool gpuMode, Mat &flowx, Mat &flowy) {
  if (expectImgPath.empty()) cout << "Specify left image path\n";
  if (targetImgPath.empty()) cout << "Specify right image path\n";
  if (expectImgPath.empty() || targetImgPath.empty()) return -1;

  Mat expectImg = imread(expectImgPath, IMREAD_GRAYSCALE);
  Mat targetImg = imread(targetImgPath, IMREAD_GRAYSCALE);
  if (expectImg.empty()) cout << "Can't open '" << expectImgPath << "'\n";
  if (targetImg.empty()) cout << "Can't open '" << targetImgPath << "'\n";
  if (expectImg.empty() || targetImg.empty()) return -1;

  if (expectImg.rows != targetImg.rows || expectImg.cols != targetImg.cols) {
    Mat resizedTargetImg(expectImg.rows, expectImg.cols, expectImg.type());
    cv::resize(targetImg, resizedTargetImg, resizedTargetImg.size(), cv::INTER_NEAREST);
    targetImg = resizedTargetImg;
  }

  GpuMat d_frameL(expectImg), d_targetImg(targetImg);
  GpuMat d_flowx, d_flowy;
  FarnebackOpticalFlow d_calc;
  Mat flowxy;

  int64 tc0, tc1;

  d_calc.pyrScale = 0.5;
  d_calc.numLevels = 3;
  d_calc.winSize = 30;
  d_calc.numIters = 3;
  d_calc.polyN = 7;
  d_calc.polySigma = 1.5;
  d_calc.flags = OPTFLOW_FARNEBACK_GAUSSIAN;

  if (gpuMode) {
    tc0 = getTickCount();
    d_calc(d_frameL, d_targetImg, d_flowx, d_flowy);
    tc1 = getTickCount();
    d_flowx.download(flowx);
    d_flowy.download(flowy);
  } else {
    tc0 = getTickCount();
    calcOpticalFlowFarneback(
                expectImg, targetImg, flowxy, d_calc.pyrScale, d_calc.numLevels, d_calc.winSize,
                d_calc.numIters, d_calc.polyN, d_calc.polySigma, d_calc.flags);
    tc1 = getTickCount();

    Mat planes[] = {flowx, flowy};
    split(flowxy, planes);
    flowx = planes[0]; flowy = planes[1];
  }

  return (tc1-tc0)/getTickFrequency();
}
