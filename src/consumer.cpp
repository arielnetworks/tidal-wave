#include <uv.h>
#include "consumer.h"

#ifdef USE_GPU
#include "opencv2/gpu/gpu.hpp"
using namespace cv::gpu;
#endif

using namespace std;

namespace tidalwave {
  Consumer::Consumer(int id, MessageQueue<Request> &reqq, MessageQueue<Response> &resq)
      : id(id), requestQueue(reqq), responseQueue(resq), isRunning(false) {

    bool gpuMode = false;

    // USE_GPUが有効で、GPUが利用可能なデバイスの数よりもidが小さい場合に、GPUを利用したOpticalFlowByGPUのインスタンスを生成する
#ifdef USE_GPU
    cout << "device count: " << cv::gpu::getCudaEnabledDeviceCount() << endl;
    int devCount = cv::gpu::getCudaEnabledDeviceCount();
    if (id < devCount) {
      cv::gpu::setDevice(id);
      gpuMode = true;
    }
#endif

    if (gpuMode) {
#ifdef USE_GPU
      opticalFlow = new OpticalFlowByGPU();
#else
      opticalFlow = new OpticalFlowByCPU();
#endif
    } else {
      opticalFlow = new OpticalFlowByCPU();
    }
  }

  Consumer::~Consumer() {
    delete opticalFlow;
  }

  int Consumer::run() {
    while (isRunning) {

      // キューから依頼を取得して、OpticalFlow処理を実行する
      Request req;
      if (requestQueue.tryPop(req)) {

        std::cout << "consume: " << req.expect_image << " <-> " << req.target_image << std::endl;
        cv::Mat flowx;
        cv::Mat flowy;

        // OpticalFlowを実行
        OpticalFlowStatus status = opticalFlow->calculate(req.expect_image, req.target_image, parameter, flowx, flowy);

        // 解析結果をレスポンスキューに入れる
        Response res;
        if (status.code == OK) {
          for (int y = 0; y < flowx.rows; ++y) {
            if (y % req.span != 0) continue;
            for (int x = 0; x < flowx.cols; ++x) {
              if (x % req.span != 0) continue;
              float dx = flowx.at<float>(y, x);
              float dy = flowy.at<float>(y, x);
              float len = (dx * dx) + (dy * dy);
              if (len > (req.threshold * req.threshold)) {
                Vector v;
                v.x = x;
                v.y = y;
                v.dx = dx;
                v.dy = dy;
                res.vectors.push_back(v);
              }
            }
          }
          res.status = res.vectors.size() == 0 ? "OK" : "SUSPICIOUS";
          res.time = status.time;
          res.expect_image = req.expect_image;
          res.target_image = req.target_image;
          res.span = req.span;
          res.threshold = req.threshold;
          cout << "finish optical flow: " << status.time << endl;
        } else {
          res.status = "ERROR";
          res.reason = status.message;
        }
        responseQueue.push(res);
      }
    }
    cout << "finish consumer" << id << endl;
    return 0;
  }

  void Consumer::start(const OpticalFlowParameter &param) {
    this->parameter = param;
    isRunning = true;
    Thread::start();
  }

  int Consumer::stop() {
    isRunning = false;
    return Thread::stop();
  }
}
