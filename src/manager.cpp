#include <node.h>
#include <unistd.h>
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

  Manager::Manager(Observer<Response> *emitter, int consumer_num)
      : async(), requestQueue(), responseQueue(), messageBuffer(), consumers(), callback_count(0), emitter(emitter) {

    // ConsumerThreadの生成
    // Dispatcherのインスタンスは最後まで解放されないので、ここでつくったConsumerのインスタンスも解放しない。
    for (int i = 0; i < consumer_num; i++) {
      Consumer *cons = new Consumer(i, requestQueue, responseQueue);
      consumers.push_back(cons);
    }
  }

  int Manager::request(const Parameter &param) {
    cout << "=== start consumer-producer test ===" << endl;

    callback_count = 0;
    requestQueue.reset();

    // asyncを使って通知するとonNextHandleが呼ばれる
    uv_async_init(uv_default_loop(), &async, notifyHandler);
    async.data = (void *) this;

    workData.parameter = param;
    workData.manager = this;
    workDataContainer.data = &workData;

    // workHandleを別スレッドで実行。完了したらonCompletedHandlerが呼ばれる
    uv_queue_work(uv_default_loop(), &workDataContainer, workHandler, finishHandler);

    cout << "=== end consumer-producer test ===" << endl;

    return 0;
  }

  void Manager::work(Parameter parameter) {
    for (vector<Consumer *>::iterator it = consumers.begin(); it != consumers.end(); it++) {
      Consumer *cons = *it;
      cons->start(parameter.optParam);
    }

    Producer producer(requestQueue);
    int request_count = producer.run(parameter);

    int finished_count = 0;
    cout << "request count : " << request_count << endl;
    // 画像比較を依頼した数と処理した数が一致するまで繰り返す
    while (request_count != finished_count) {
      // キューからレスポンスを取得
      Response res;
      if (responseQueue.tryPop(res)) {
        res.threshold = parameter.threshold;
        res.span = parameter.span;
        messageBuffer.push(res);
        // sendCallback処理を実行してもらうように通知する
        uv_async_send(&async);
        finished_count++;
      }
      cout << "request: " << request_count << ", finished: " << finished_count << endl;
    }

    // ConsumerThreadに完了を通知して、終了するまで待つ
    requestQueue.stop();
    for (vector<Consumer *>::iterator it = consumers.begin(); it != consumers.end(); it++) {
      Consumer *cons = *it;
      cons->stop();
    }

    // すべてのsendCallback処理が完了するまで待つ。
    // producerThreadが完了するとすぐにafterが呼ばれて、mutexとかいろいろ解放されてしまうので。
    while (true) {
      if (request_count == callback_count) {
        break;
      }
      cout << "request: " << request_count << ", callback: " << callback_count << endl;
      sleep(1);
    }
  }

  // Node.js側のコールバック関数を呼び出してオプティカルフローの計算結果を通知する
  // uv_async_sendを複数回呼んでもsendCallbackは1回にまとめられることがあるので、msg_bufferで複数のデータをやりとり。
  void Manager::notify() {
    HandleScope scope;

    cout << "onNext!" << endl;
    vector<Response> responses = messageBuffer.popAll();
    for (vector<Response>::iterator it = responses.begin(); it != responses.end(); it++) {
      Response res_buf = *it;

      if(res_buf.status == "ERROR"){
        emitter->onError(res_buf.reason);
      } else {
        emitter->onNext(res_buf);
      }

      callback_count++;
      cout << "callback_count: " << callback_count << endl;
    }

  };

  // すべての処理が完了した時に呼び出される
  void Manager::finish() {
    // Node.js側に終了のコールバック
    emitter->onCompleted();
    uv_close((uv_handle_t *) &async, NULL);
  }

  void Manager::workHandler(uv_work_t *req) {
    WorkData &data = *static_cast<WorkData *>(req->data);
    data.manager->work(data.parameter);
  }
  void Manager::notifyHandler(uv_async_t *handle, int status) {
    Manager &self = *static_cast<Manager *>(handle->data);
    self.notify();
  }
  void Manager::finishHandler(uv_work_t *req, int status) {
    WorkData &data = *static_cast<WorkData *>(req->data);
    data.manager->finish();
  }
}

