#ifndef OBSERVER_H
#define OBSERVER_H

#include <iostream>
#include <vector>
#include <sstream>
#include <opencv2/core/core.hpp>

namespace tidalwave {
  template<typename TValue, typename TError, typename TReport>
  class Observer {
  public:
    virtual void onNext(const TValue &value) = 0;

    virtual void onError(const TError &err) = 0;

    virtual void onCompleted(const TReport &report) = 0;
  };
}
#endif // OBSERVER_H
