#ifndef OBSERVER_H
#define OBSERVER_H

#include <iostream>
#include <vector>
#include <sstream>
#include <opencv2/core/core.hpp>

namespace tidalwave{
  template<typename T>
  class Observer {
  public:
    virtual void onNext(const T& value) = 0;
    virtual void onError(const std::string &reason) = 0;
    virtual void onCompleted() = 0;
  };
}
#endif // OBSERVER_H
