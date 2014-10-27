#ifndef MANAGER_H
#define MANAGER_H

#include "observer.h"
#include "producer.h"
#include "message_buffer.h"

// CPUのコア数とGPUのデバイス数の合計に設定すると最もパフォーマンスがよい
#define MAX_CONSUMERS 5

namespace tidalwave {
  class Manager;

  struct WorkData {
    Parameter parameter;
    Manager *manager;
  };

  /*
   * @brief Node.js側からの依頼に応じて、各スレッドを立ち上げたりコールバックを設定したり終了処理をおこなったりするクラス
   */
  class Manager {
  public:
    Manager(Observer<Response> *emitter, int consumer_num = MAX_CONSUMERS);

    /*
     * @brief Node.jsからの依頼に応じて処理を開始する
     * @param param OpticalFlowの実行に必要なパラメータ
     */
    int request(const Parameter &param);

    /*
     * @brief 各スレッドを立ち上げ、終了するまで待つ。
     * スレッドとして起動
     */
    void work(Parameter parameter);

    /*
     * @brief オプティカルフローの処理結果をNode.js側に通知する
     * 1つの画像の解析が完了するたびに呼び出される
     */
    void notify();

    /*
     * @brief すべてのオプティカルフローの処理が完了したことをNode.js側に通知する
     */
    void finish();


    static void workHandler(uv_work_t *req);
    static void notifyHandler(uv_async_t *handle, int status);
    static void finishHandler(uv_work_t *req, int status);

  private:
    uv_async_t async;

    MessageQueue<Request> requestQueue;
    MessageQueue<Response> responseQueue;
    MessageBuffer<Response> messageBuffer;

    std::vector<Consumer *> consumers;

    int callback_count;

    WorkData workData;
    uv_work_t workDataContainer;
    Observer<Response> *emitter;
  };
}
#endif // MANAGER_H
