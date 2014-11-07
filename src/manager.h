#ifndef MANAGER_H
#define MANAGER_H

#include "observer.h"
#include "message_buffer.h"
#include "message_queue.h"
#include "consumer.h"

namespace tidalwave {

  struct Parameter {
    double threshold;
    int span;
    int numThreads;
    OpticalFlowParameter optParam;
  };
  /*
   * @brief Node.js側からの依頼に応じて、各スレッドを立ち上げたりコールバックを設定したり終了処理をおこなったりするクラス
   */
  class Manager {
  public:
    Manager(Observer<Response, std::string, Report> *emitter);
    virtual ~Manager();

    /*
     * @brief スレッドを立ち上げてリクエストを受付可能状態にする
     * @param param OpticalFlowの実行に必要なパラメータ
     */
    int start(const Parameter &param);

    void stop();

    /*
     * @brief Node.jsからの依頼に応じて処理を開始する
     * @param req 比較する画像
     */
    int request(const std::string &expect_image, const std::string &target_image);

    /*
     * @brief 各スレッドを立ち上げ、終了するまで待つ。
     * スレッドとして起動
     */
    void work();

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

    bool isRunning;
  private:
    uv_async_t async;

    MessageQueue<Request> requestQueue;
    MessageQueue<Response> responseQueue;
    MessageBuffer<Response> messageBuffer;
    MessageBuffer<std::string> errorBuffer;

    std::vector<Consumer *> consumers;

    Parameter param;
    Report report;

    uv_work_t workDataContainer;

    // 参照を持ってるだけなので解放しなくてよい。
    Observer<Response, std::string, Report> *emitter;
  };
}
#endif // MANAGER_H
