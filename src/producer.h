#ifndef PRODUCER_H
#define PRODUCER_H

#include "observer.h"
#include "message_queue.h"
#include "consumer.h"
#include "message_buffer.h"

namespace tidalwave {
  struct Parameter {
    std::string expect_path;
    std::string target_path;
    double threshold;
    int span;
    OpticalFlowParameter optParam;
  };

  /*
   * @brief オプティカルフロー処理の依頼を出すクラス
   */
  class Producer {

  public:

    /*
     * @param reqq 処理のリクエストを出すためのメッセージキュー
     */
    Producer(MessageQueue<Request> &reqq);

    /*
     * @brief 指定された2つのディレクトリの中からパスが一致する画像の組を作って、画像比較処理の依頼をおこなう
     */
    int run(const Parameter &param);

  private:
    MessageQueue<Request> &requestQueue;

  };
}
#endif // PRODUCER_H
