#ifndef CONSUMER_H
#define CONSUMER_H

#include "thread.h"
#include "message_queue.h"
#include "opticalflow.h"


namespace tidalwave {

  /*
   * @brief 新しいスレッドを生成し、そこでオプティカルフロー演算をおこなうクラス
   */
  class Consumer : public Thread {

  public:
    /*
     * @param id コンシューマのID
     * @param reqq 処理のリクエストを受け取るためのメッセージキュー
     * @param resq 処理結果を送信するためのメッセージキュー
     */
    Consumer(int id, MessageQueue<Request> &reqq, MessageQueue<Response> &resq);

    virtual ~Consumer();

    /*
     * @brief 別スレッドとして実行する処理
     */
    virtual int run();

    /*
     * @brief スレッドを立ち上げる
     */
    void start(const OpticalFlowParameter &param);

    /*
     * @brief スレッドを止める
     * runの処理が完了するまで待つ
     */
    virtual int stop();

  private:
    int id;
    MessageQueue<Request> &requestQueue;
    MessageQueue<Response> &responseQueue;

    bool isRunning;
    OpticalFlowParameter parameter;
    OpticalFlow *opticalFlow;
  };
}

#endif // CONSUMER_H
