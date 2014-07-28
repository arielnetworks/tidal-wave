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


float calc_opticalflow(string pathL, string pathR, bool gpuMode, Mat &flowx, Mat &flowy)
{
    if (pathL.empty()) cout << "Specify left image path\n";
    if (pathR.empty()) cout << "Specify right image path\n";
    if (pathL.empty() || pathR.empty()) return -1;

    Mat frameL = imread(pathL, IMREAD_GRAYSCALE);
    Mat frameR = imread(pathR, IMREAD_GRAYSCALE);
    if (frameL.empty()) cout << "Can't open '" << pathL << "'\n";
    if (frameR.empty()) cout << "Can't open '" << pathR << "'\n";
    if (frameL.empty() || frameR.empty()) return -1;

    GpuMat d_frameL(frameL), d_frameR(frameR);
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
        d_calc(d_frameL, d_frameR, d_flowx, d_flowy);
        tc1 = getTickCount();
        d_flowx.download(flowx);
        d_flowy.download(flowy);
    } else {
        tc0 = getTickCount();
        calcOpticalFlowFarneback(
                    frameL, frameR, flowxy, d_calc.pyrScale, d_calc.numLevels, d_calc.winSize,
                    d_calc.numIters, d_calc.polyN, d_calc.polySigma, d_calc.flags);
        tc1 = getTickCount();

        Mat planes[] = {flowx, flowy};
        split(flowxy, planes);
        flowx = planes[0]; flowy = planes[1];
    }

    return (tc1-tc0)/getTickFrequency();
}
