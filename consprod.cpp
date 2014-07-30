#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include <iostream>
#include <vector>
#include <sstream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/gpu/gpu.hpp"

#include <node.h>
#include <v8.h>
#include "uv.h"

 
#define MAX_CONSUMERS 3
#define MAX_LOOPS 1

using namespace v8;
using namespace std;
 
bool traverse(const string filepath, const int maxdepth, int curdepth, vector<string> &list);

typedef struct {
  ngx_queue_t queue;
  int data;
  char expect_path[255];
  char target_path[255];
} request_buffer;

typedef struct {
  ngx_queue_t queue;
  cv::Mat *flowx;
  cv::Mat *flowy;
} response_buffer;
 
static ngx_queue_t req_queue;
static ngx_queue_t res_queue;

static uv_mutex_t req_mutex;
static uv_mutex_t res_mutex;
static uv_cond_t empty;
static uv_cond_t full;

static volatile int request_number = 0;
static volatile int finished_requests = 0;
static volatile bool isRunning = true;

 
 
 
typedef struct {
  string expect_path;
  string target_path;
} producer_arg;

typedef struct {
  int num;
} consumer_arg;

float calc_opticalflow(string pathL, string pathR, bool gpuMode, cv::Mat &flowx, cv::Mat &flowy);

static void copyToNode(const cv::Mat *flowx, const cv::Mat *flowy, const double threshold, const int span, const Local<Array> &results) {
    Persistent<String> x_symbol = NODE_PSYMBOL("x");
    Persistent<String> y_symbol = NODE_PSYMBOL("y");
    Persistent<String> dx_symbol = NODE_PSYMBOL("dx");
    Persistent<String> dy_symbol = NODE_PSYMBOL("dy");
    Persistent<String> len_symbol = NODE_PSYMBOL("len");

    int i = 0;
    for (int y = 0; y < flowx->rows; ++y) {
        if ( y % span != 0 ) continue;
        for (int x = 0; x < flowx->cols; ++x) {
            if ( x % span != 0 ) continue;
            float dx = flowx->at<float>(y,x);
            float dy = flowy->at<float>(y,x);
            float len = (dx*dx) + (dy*dy);
            if (len > (threshold*threshold)) {
                Local<Object> obj = Object::New();
                obj->Set(x_symbol, Integer::New(x));
                obj->Set(y_symbol, Integer::New(y));
                obj->Set(dx_symbol, Number::New(dx));
                obj->Set(dy_symbol, Number::New(dy));
                //obj->Set(len_symbol, Number::New(len));
                results->Set(i++, obj);
            }
        }
    }
}

static void producer(void* arg) {
  producer_arg *parg = (producer_arg*)arg;

  vector<string> expect_files;
  vector<string> target_files;

  traverse(parg->expect_path, 0, 1, expect_files);
  traverse(parg->target_path, 0, 1, target_files);

  for(vector<string>::iterator tit = target_files.begin(); tit != target_files.end(); ++tit) {
    for(vector<string>::iterator eit = expect_files.begin(); eit != expect_files.end(); ++eit) {
      string expect = (*eit).substr(parg->expect_path.length()+1);
      string target = (*tit).substr(parg->target_path.length()+1);
      if (target == expect) {
        uv_mutex_lock(&req_mutex);
        request_buffer* buf;
        buf = (request_buffer*)malloc(sizeof(request_buffer));
        ngx_queue_init(&buf->queue);
        strcpy(buf->expect_path, (*eit).c_str());
        strcpy(buf->target_path, (*tit).c_str());
        ngx_queue_insert_tail(&req_queue, &buf->queue);
        uv_mutex_unlock(&req_mutex);
        uv_cond_signal(&full);
        request_number++;
        break;
      }
    }
  }
  cout << "finish producer" << endl;
}
 
static void consumer(void* arg) {
  consumer_arg *carg = (consumer_arg*)arg;

  while(isRunning) {
    uv_mutex_lock(&req_mutex);
    while(ngx_queue_empty(&req_queue) && isRunning) {
      uv_cond_wait(&full, &req_mutex);
    }
    if(ngx_queue_empty(&req_queue) || !isRunning) {
      uv_mutex_unlock(&req_mutex);
      break;
    }

    ngx_queue_t *q;
    request_buffer *buf;
    assert(!ngx_queue_empty(&req_queue));
    q = ngx_queue_last(&req_queue);
    ngx_queue_remove(q);
 
    buf = ngx_queue_data(q, request_buffer, queue);
    string expect_path = string(buf->expect_path);
    string target_path = string(buf->target_path);

    uv_mutex_unlock(&req_mutex);


    cv::Mat *flowx = new cv::Mat();
    cv::Mat *flowy = new cv::Mat();
    cout << "opticalflow: " << expect_path << ":" << target_path << endl;
    float t = calc_opticalflow(expect_path, target_path, false, *flowx, *flowy);

cout << "finish opticalflow " << t << endl;

    free(buf);
    
    uv_mutex_lock(&res_mutex);
    response_buffer* res_buf;
    res_buf = (response_buffer*)malloc(sizeof(response_buffer));
    ngx_queue_init(&res_buf->queue);
    res_buf->flowx = flowx;
    res_buf->flowy = flowy;
    ngx_queue_insert_tail(&res_queue, &res_buf->queue);
    uv_cond_signal(&empty);
    uv_mutex_unlock(&res_mutex);

    finished_requests++;
  }
  cout << "finish consumer"  << carg->num << endl;
}
 
int consprod(const string expect_path, const string target_path, const double threshold, const int span, Local<Function> &cb) {
  int i;
  uv_thread_t cthreads[MAX_CONSUMERS];
  uv_thread_t pthread;

request_number = 0;
finished_requests = 0;
isRunning = true;
 
  fprintf(stdout, "=== start consumer-producer test ===\n");
  ngx_queue_init(&req_queue);
  ngx_queue_init(&res_queue);

  assert(0 == uv_mutex_init(&req_mutex));
  assert(0 == uv_mutex_init(&res_mutex));
  assert(0 == uv_cond_init(&empty));
  assert(0 == uv_cond_init(&full));
 
  for(i = 0; i < MAX_CONSUMERS; i++) {
    consumer_arg *carg = new consumer_arg();
    carg->num = i;
    assert(0 == uv_thread_create(&cthreads[i], consumer, (void*)carg));
  }
 
  producer_arg parg;
  parg.expect_path = expect_path;
  parg.target_path = target_path;

  assert(0 == uv_thread_create(&pthread, producer, (void*)&parg));
  assert(0 == uv_thread_join(&pthread));


  int push_count = 0;
  cout << "request number : " << request_number << endl;
  while(request_number != push_count){
    while(ngx_queue_empty(&res_queue)) {
      uv_cond_wait(&empty, &res_mutex);
    }
    push_count++;
    cout << "pop!!" << endl;

    ngx_queue_t *q;
    response_buffer *buf;
    assert(!ngx_queue_empty(&res_queue));
    q = ngx_queue_last(&res_queue);
    ngx_queue_remove(q);
 
    buf = ngx_queue_data(q, response_buffer, queue);
    Local<Array> results = Array::New();
    copyToNode(buf->flowx, buf->flowy, threshold, span, results);

    const unsigned argc = 1;
    Local<Value> argv[argc] = { Local<Value>::New(results) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);

    delete buf->flowx;
    delete buf->flowy;
    free(buf);
  }


  isRunning = false;

  uv_cond_broadcast(&full);

  for(i = 0; i < MAX_CONSUMERS; i++) {
    assert(0 == uv_thread_join(&cthreads[i]));
  }

  
  uv_cond_destroy(&empty);
  uv_cond_destroy(&full);
  uv_mutex_destroy(&req_mutex);
  uv_mutex_destroy(&res_mutex);
 
  fprintf(stdout, "=== end consumer-producer test ===\n");
  return 0;
}
