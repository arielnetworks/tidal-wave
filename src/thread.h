#ifndef THREAD_H
#define THREAD_H

#include <uv.h>

namespace tidalwave {

  /*
   * スレッドを扱いやすくするためのクラス
   */
  class Thread {
  public:
    Thread();

    virtual ~Thread();

    /*
     * @brief スレッドとして実行される関数
     * オーバーライドして使ってね
     */
    virtual int run();

    /*
     * @brief スレッドの開始
     */
    virtual void start();

    /*
     * @brief スレッドの修了
     */
    virtual int stop();

    /*
     * クラスのインスタンス関数をスレッドとして実行するために経由する関数
     */
    static void runHandler(void *args);

  private:
    uv_thread_t thread;
  };
};

#endif // THREAD_H
