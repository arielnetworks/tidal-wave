#include "thread.h"
#include <iostream>

namespace tidalwave {
  Thread::Thread() {
  }

  Thread::~Thread() {
  }

  int Thread::run() {
    return 0;
  }

  void Thread::start() {
    uv_thread_create(&thread, (void (*)(void *)) Thread::runHandler, (void *) this);
  }

  int Thread::stop() {
    uv_thread_join(&thread);
    return 0;
  }

  void Thread::runHandler(void *args) {
    Thread &t = *static_cast<Thread *>(args);
    t.run();
  }
};


