#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <uv.h>
#include <string.h>
#include <assert.h>
#include <queue>
#include "opencv2/core/core.hpp"
#include "observer.h"

namespace tidalwave {

  struct Request {
    std::string expect_image;
    std::string target_image;
  };

  struct Response {
    cv::Mat flowx;
    cv::Mat flowy;
    std::string expect_image;
    std::string target_image;
    float time;
    double threshold;
    int span;

    std::string status;
    std::string reason;
  };


  template<typename T>
  class MessageQueue {

  public:
    MessageQueue()
        : mutex(), notifier(), isRunning(true), queue() {

      assert(0 == uv_mutex_init(&mutex));
      assert(0 == uv_cond_init(&notifier));
    }

    /*
     * @brief キューからメッセージを取得する
     * データが取得できるか、終了通知が送られるまで待つ
     * @param buf 取得したデータ
     * @return データが取得できたらtrue, 終了通知によって抜けた場合はfalse
     */
    bool tryPop(T &buf) {

      uv_mutex_lock(&mutex);
      // キューにリクエストがくるか、終了通知がくるまで待つ
      while (queue.empty() && isRunning) {
        uv_cond_wait(&notifier, &mutex);
      }
      // 終了通知がきたらループを抜ける
      if (queue.empty() || !isRunning) {
        uv_mutex_unlock(&mutex);
        return false;
      }

      // リクエストをキューから取得
      buf = queue.front();
      queue.pop();
      uv_mutex_unlock(&mutex);
      return true;
    }

    void push(const T &buf) {
      uv_mutex_lock(&mutex);
      queue.push(buf);
      uv_mutex_unlock(&mutex);
      uv_cond_signal(&notifier);
    }

    void stop() {
      std::cout << "stop queue" << std::endl;
      isRunning = false;
      uv_cond_broadcast(&notifier);
    }

    /*
     * @brief 一旦stopしたら、再度利用する場合はresetを呼んでください
     */
    void reset() {
      isRunning = true;
    }

  private:
    uv_mutex_t mutex;
    uv_cond_t notifier;

    bool isRunning;
    std::queue<T> queue;

    MessageQueue(const MessageQueue &) {
    };

    MessageQueue &operator=(const MessageQueue &) {
    };
  };
}
#endif // MESSAGE_QUEUE_H
