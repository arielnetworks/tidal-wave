#include <iostream>
#include <node.h>
#include "broker.h"

using namespace v8;
using namespace node;
using namespace tidalwave;

void init(Handle<Object> target) {
  Broker::initialize(target);
}

NODE_MODULE(tidalwave, init)
