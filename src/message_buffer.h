#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H

#include <uv.h>
#include <assert.h>
#include <vector>

namespace tidalwave {
  template<typename T>
  class MessageBuffer {
  public:
    MessageBuffer() {
      assert(0 == uv_mutex_init(&mutex));
    }

    ~MessageBuffer() {
      uv_mutex_destroy(&mutex);
    }

    void push(const T &msg) {
      // sendCallback向けにデータを詰める
      uv_mutex_lock(&mutex);
      buffer.push_back(msg);
      uv_mutex_unlock(&mutex);
    }

    std::vector<T> popAll() {

      uv_mutex_lock(&mutex);
      std::vector<T> ret = buffer;
      buffer.clear();
      uv_mutex_unlock(&mutex);

      return ret;
    }

  private:

    MessageBuffer(const MessageBuffer &) {
    };

    MessageBuffer &operator=(const MessageBuffer &) {
    };
    std::vector<T> buffer;
    uv_mutex_t mutex;
  };
}

#endif // MESSAGE_BUFFER_H
