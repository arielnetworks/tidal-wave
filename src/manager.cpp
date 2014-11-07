#include <node.h>
#include <iostream>
#include <vector>

#include "opencv2/core/core.hpp"
#include "opencv2/video/video.hpp"

#ifdef USE_GPU
#include "opencv2/gpu/gpu.hpp"
#endif

#include "manager.h"

using namespace v8;
using namespace std;

namespace tidalwave {

  Manager::Manager(Observer<Response, string, Report> *emitter)
      : isRunning(true),
        async(),
        requestQueue(),
        responseQueue(),
        messageBuffer(),
        errorBuffer(),
        consumers(),
        report(),
        emitter(emitter) {

  }

  Manager::~Manager() {
    for (vector<Consumer *>::iterator it = consumers.begin(); it != consumers.end(); it++) {
      Consumer *cons = *it;
      delete cons;
    }
    consumers.clear();
  }

  int Manager::start(const Parameter &param) {
    this->param = param;
    requestQueue.reset();
    responseQueue.reset();

    // asyncを使って通知するとnotifyHandlerが呼ばれる
    uv_async_init(uv_default_loop(), &async, notifyHandler);
    async.data = (void *) this;

    workDataContainer.data = this;

    // workHandleを別スレッドで実行。完了したらfinishHandlerが呼ばれる
    uv_queue_work(uv_default_loop(), &workDataContainer, workHandler, finishHandler);

    // ConsumerThreadの生成
    for (int i = 0; i < param.numThreads; i++) {
      Consumer *cons = new Consumer(i, requestQueue, responseQueue);
      cons->start(param.optParam);
      consumers.push_back(cons);
    }
    return 0;
  }

  void Manager::stop() {
    isRunning = false;
    responseQueue.stop();
  }

  int Manager::request(const string &expect_image, const string &target_image) {
    Request req;
    req.expect_image = expect_image;
    req.target_image = target_image;
    req.span = param.span;
    req.threshold = param.threshold;
    std::cout << "request: " << expect_image << " <-> " << target_image << endl;
    requestQueue.push(req);
    report.requestCount++;
    return 0;
  }

  void Manager::work() {

    while (isRunning) {
      // キューからレスポンスを取得
      Response res;
      if (responseQueue.tryPop(res)) {
        messageBuffer.push(res);
        // sendCallback処理を実行してもらうように通知する
        uv_async_send(&async);
      }
    }

    // ConsumerThreadに完了を通知して、終了するまで待つ
    requestQueue.stop();
    for (vector<Consumer *>::iterator it = consumers.begin(); it != consumers.end(); it++) {
      Consumer *cons = *it;
      cons->stop();
    }
  }

  // Node.js側のコールバック関数を呼び出してオプティカルフローの計算結果を通知する
  // uv_async_sendを複数回呼んでもsendCallbackは1回にまとめられることがあるので、msg_bufferで複数のデータをやりとり。
  void Manager::notify() {
    HandleScope scope;

    vector<Response> responses = messageBuffer.popAll();
    for (vector<Response>::iterator it = responses.begin(); it != responses.end(); it++) {
      Response res_buf = *it;

      if (res_buf.status == "ERROR") {
        report.errorCount++;
        emitter->onError(res_buf.reason);
      } else {
        report.dataCount++;
        emitter->onNext(res_buf);
      }
    }

    vector<string> errors = errorBuffer.popAll();
    for (vector<string>::iterator it = errors.begin(); it != errors.end(); it++) {
      report.errorCount++;
      string reason = *it;
      emitter->onError(reason);
    }

  };

  // すべての処理が完了した時に呼び出される
  void Manager::finish() {
    // Node.js側に終了のコールバック
    emitter->onCompleted(report);

    uv_close((uv_handle_t *) &async, NULL);
  }

  void Manager::workHandler(uv_work_t *req) {
    Manager &manager = *static_cast<Manager *>(req->data);
    manager.work();
  }

  void Manager::notifyHandler(uv_async_t *handle, int status) {
    Manager &manager = *static_cast<Manager *>(handle->data);
    manager.notify();
  }

  void Manager::finishHandler(uv_work_t *req, int status) {
    Manager &manager = *static_cast<Manager *>(req->data);
    manager.finish();
  }
}

