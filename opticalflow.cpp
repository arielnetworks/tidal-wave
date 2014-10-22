#include <iostream>
#include <vector>
#include <sstream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"

using namespace std;
using namespace cv;

#ifdef USE_GPU
#include "opencv2/gpu/gpu.hpp"
using namespace cv::gpu;
#endif

float calcOpticalFlow(string expectImgPath, string targetImgPath, bool gpuMode, Mat &flowx, Mat &flowy) {
  if (expectImgPath.empty()) cout << "Specify left image path\n";
  if (targetImgPath.empty()) cout << "Specify right image path\n";
  if (expectImgPath.empty() || targetImgPath.empty()) return -1;

  Mat expectImg = imread(expectImgPath, IMREAD_GRAYSCALE);
  Mat targetImg = imread(targetImgPath, IMREAD_GRAYSCALE);
  if (expectImg.empty()) cout << "Can't open '" << expectImgPath << "'\n";
  if (targetImg.empty()) cout << "Can't open '" << targetImgPath << "'\n";
  if (expectImg.empty() || targetImg.empty()) return -1;

  if (abs(expectImg.rows - targetImg.rows) > 5 || abs(expectImg.cols - targetImg.cols) > 5) {
    return -1;
  }

  if (expectImg.rows != targetImg.rows || expectImg.cols != targetImg.cols) {
    Mat resizedTargetImg(expectImg.rows, expectImg.cols, expectImg.type());
    cv::resize(targetImg, resizedTargetImg, resizedTargetImg.size(), cv::INTER_NEAREST);
    targetImg = resizedTargetImg;
  }

  double pyrScale = 0.5;
  int numLevels = 3;
  int winSize = 30;
  int numIters = 3;
  int polyN = 7;
  double polySigma = 1.5;
  int flags = OPTFLOW_FARNEBACK_GAUSSIAN;

#ifdef USE_GPU
  GpuMat d_frameL(expectImg), d_targetImg(targetImg);
  GpuMat d_flowx, d_flowy;
  FarnebackOpticalFlow d_calc;

  d_calc.pyrScale = pyrScale;
  d_calc.numLevels = numLevels;
  d_calc.winSize = winSize;
  d_calc.numIters = numIters;
  d_calc.polyN = polyN;
  d_calc.polySigma = polySigma;
  d_calc.flags = flags;
#endif

  Mat flowxy;

  int64 tc0, tc1;


  if (gpuMode) {
#ifdef USE_GPU
    tc0 = getTickCount();
    d_calc(d_frameL, d_targetImg, d_flowx, d_flowy);
    tc1 = getTickCount();
    d_flowx.download(flowx);
    d_flowy.download(flowy);
#endif
  } else {
    tc0 = getTickCount();
    calcOpticalFlowFarneback(
                expectImg, targetImg, flowxy, pyrScale, numLevels, winSize,
                numIters, polyN, polySigma, flags);
    tc1 = getTickCount();

    Mat planes[] = {flowx, flowy};
    split(flowxy, planes);
    flowx = planes[0]; flowy = planes[1];
  }

  return (tc1-tc0)/getTickFrequency();
}
