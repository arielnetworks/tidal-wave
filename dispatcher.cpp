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

// CPUのコア数とGPUのデバイス数の合計に設定すると最もパフォーマンスがよい
#define MAX_CONSUMERS 5

using namespace v8;
using namespace std;
 
bool listFiles(const string filepath, int curdepth, vector<string> &list);

typedef struct {
  ngx_queue_t queue;
  int data;
  char expect_image[255];
  char target_image[255];
} RequestBuffer;

typedef struct {
  ngx_queue_t queue;
  cv::Mat *flowx;
  cv::Mat *flowy;
  char expect_image[255];
  char target_image[255];
  float time;
} ResponseBuffer;

typedef struct {
  string expect_path;
  string target_path;
} ProducerArg;

typedef struct {
  int num;
} ConsumerArg;

static ngx_queue_t req_queue;
static ngx_queue_t res_queue;

static uv_mutex_t req_mutex;
static uv_mutex_t res_mutex;
static uv_cond_t responseNotification;
static uv_cond_t requestNotification;

static volatile int request_count = 0;
static volatile bool isRunning = true;

float calcOpticalFlow(string pathL, string pathR, bool gpuMode, cv::Mat &flowx, cv::Mat &flowy);

// 解析結果をNode.jsで扱える型で生成
static void createResult(
  const string expect_image, const string target_image,
  const cv::Mat *flowx, const cv::Mat *flowy,
  const double threshold, const int span,
  const float time,
  const Local<Object> &result) {

  Persistent<String> time_symbol = NODE_PSYMBOL("time");
  Persistent<String> status_symbol = NODE_PSYMBOL("status");

  Persistent<String> span_symbol = NODE_PSYMBOL("span");
  Persistent<String> threshold_symbol = NODE_PSYMBOL("threshold");
  Persistent<String> expect_symbol = NODE_PSYMBOL("expect_image");
  Persistent<String> target_symbol = NODE_PSYMBOL("target_image");

  Persistent<String> vector_symbol = NODE_PSYMBOL("vector");
  Persistent<String> x_symbol = NODE_PSYMBOL("x");
  Persistent<String> y_symbol = NODE_PSYMBOL("y");
  Persistent<String> dx_symbol = NODE_PSYMBOL("dx");
  Persistent<String> dy_symbol = NODE_PSYMBOL("dy");

  result->Set(span_symbol, Integer::New(span));
  result->Set(threshold_symbol, Number::New(threshold));
  result->Set(expect_symbol, String::New(expect_image.c_str()));
  result->Set(target_symbol, String::New(target_image.c_str()));

  result->Set(time_symbol, Number::New(time));


  Local<Array> vectors = Array::New();
  int vector_len = 0;
  for (int y = 0; y < flowx->rows; ++y) {
    if ( y % span != 0 ) continue;
    for (int x = 0; x < flowx->cols; ++x) {
      if ( x % span != 0 ) continue;
      float dx = flowx->at<float>(y,x);
      float dy = flowy->at<float>(y,x);
      float len = (dx*dx) + (dy*dy);
      if (len > (threshold*threshold)) {
        Local<Object> v = Object::New();
        v->Set(x_symbol, Integer::New(x));
        v->Set(y_symbol, Integer::New(y));
        v->Set(dx_symbol, Number::New(dx));
        v->Set(dy_symbol, Number::New(dy));
        vectors->Set(vector_len++, v);
      }
    }
  }
  result->Set(vector_symbol, vectors);

  if (time < 0) {
    result->Set(status_symbol, String::New("ERROR"));
  } else if (vector_len == 0) {
    result->Set(status_symbol, String::New("OK"));
  } else {
    result->Set(status_symbol, String::New("SUSPICIOUS"));
  } 
}

// 指定されたディレクトリをトラバースして、画像比較処理の依頼をおこなう
static void producerThread(void* arg) {
  ProducerArg *parg = (ProducerArg*)arg;

  vector<string> expect_files;
  vector<string> target_files;

  listFiles(parg->expect_path, 1, expect_files);
  listFiles(parg->target_path, 1, target_files);

  for(vector<string>::iterator tit = target_files.begin(); tit != target_files.end(); ++tit) {
    for(vector<string>::iterator eit = expect_files.begin(); eit != expect_files.end(); ++eit) {
      string expect_image = (*eit).substr(parg->expect_path.length()+1);
      string target_image = (*tit).substr(parg->target_path.length()+1);
      // expect_pathとtarget_pathの中に同じ名前のファイルが見つかったら処理を依頼
      if (target_image == expect_image) {
        uv_mutex_lock(&req_mutex);
        RequestBuffer* req_buf;
        req_buf = new RequestBuffer();
        ngx_queue_init(&req_buf->queue);
        strcpy(req_buf->expect_image, (*eit).c_str());
        strcpy(req_buf->target_image, (*tit).c_str());
        ngx_queue_insert_tail(&req_queue, &req_buf->queue);
        uv_mutex_unlock(&req_mutex);
        uv_cond_signal(&requestNotification);
        request_count++;
        break;
      }
    }
  }
  cout << "finish producer" << endl;
}

// キューから依頼を取得して、OpticalFlow処理を実行する
static void consumerThread(void* arg) {
  ConsumerArg *carg = (ConsumerArg*)arg;

  bool gpuMode = false;

  int devCount = cv::gpu::getCudaEnabledDeviceCount();
  if (carg->num < devCount) {
    cv::gpu::setDevice(carg->num);
    gpuMode = true;
  }

  while(isRunning) {
    uv_mutex_lock(&req_mutex);
    // キューにリクエストがくるか、終了通知がくるまで待つ
    while(ngx_queue_empty(&req_queue) && isRunning) {
      uv_cond_wait(&requestNotification, &req_mutex);
    }
    // 終了通知がきたらループを抜ける
    if(ngx_queue_empty(&req_queue) || !isRunning) {
      uv_mutex_unlock(&req_mutex);
      break;
    }

    // リクエストをキューから取得
    ngx_queue_t *q;
    RequestBuffer *req_buf;
    assert(!ngx_queue_empty(&req_queue));
    q = ngx_queue_last(&req_queue);
    ngx_queue_remove(q);
    req_buf = ngx_queue_data(q, RequestBuffer, queue);
    string expect_image = string(req_buf->expect_image);
    string target_image = string(req_buf->target_image);
    uv_mutex_unlock(&req_mutex);

    // OpticalFlowを実行
    cv::Mat *flowx = new cv::Mat();
    cv::Mat *flowy = new cv::Mat();
    float t = calcOpticalFlow(expect_image, target_image, gpuMode, *flowx, *flowy);

    // 解析結果をレスポンスキューに入れる
    uv_mutex_lock(&res_mutex);
    ResponseBuffer* res_buf;
    res_buf = new ResponseBuffer();
    ngx_queue_init(&res_buf->queue);
    res_buf->flowx = flowx;
    res_buf->flowy = flowy;
    res_buf->time = t;
    strcpy(res_buf->expect_image, req_buf->expect_image);
    strcpy(res_buf->target_image, req_buf->target_image);
    ngx_queue_insert_tail(&res_queue, &res_buf->queue);
    uv_cond_signal(&responseNotification);
    uv_mutex_unlock(&res_mutex);

    delete req_buf;
  }
  cout << "finish consumer"  << carg->num << endl;
}
 
int dispatch(const string expect_path, const string target_path, const double threshold, const int span, Local<Function> &cb) {

  cout << "device count: " << cv::gpu::getCudaEnabledDeviceCount() << endl;
  fprintf(stdout, "=== start consumer-producer test ===\n");

  request_count = 0;
  isRunning = true;

  ngx_queue_init(&req_queue);
  ngx_queue_init(&res_queue);
  assert(0 == uv_mutex_init(&req_mutex));
  assert(0 == uv_mutex_init(&res_mutex));
  assert(0 == uv_cond_init(&responseNotification));
  assert(0 == uv_cond_init(&requestNotification));

  // ConsumerThreadの生成
  uv_thread_t cthreads[MAX_CONSUMERS];
  for(int i = 0; i < MAX_CONSUMERS; i++) {
    ConsumerArg *carg = new ConsumerArg();
    carg->num = i;
    assert(0 == uv_thread_create(&cthreads[i], consumerThread, (void*)carg));
  }

  // ProducerThreadの生成
  uv_thread_t pthread;
  ProducerArg parg;
  parg.expect_path = expect_path;
  parg.target_path = target_path;
  assert(0 == uv_thread_create(&pthread, producerThread, (void*)&parg));

  // ProducerThreadが完了するまで待つ
  assert(0 == uv_thread_join(&pthread));

  int finished_count = 0;
  cout << "request count : " << request_count << endl;
  // 画像比較を依頼した数と処理した数が一致するまで繰り返す
  while (request_count != finished_count) {
    while (ngx_queue_empty(&res_queue)) {
      uv_cond_wait(&responseNotification, &res_mutex);
    }
    finished_count++;

    // キューからレスポンスを取得
    ngx_queue_t *q;
    ResponseBuffer *res_buf;
    assert(!ngx_queue_empty(&res_queue));
    q = ngx_queue_last(&res_queue);
    ngx_queue_remove(q);
    res_buf = ngx_queue_data(q, ResponseBuffer, queue);
    string expect_image = string(res_buf->expect_image);
    string target_image = string(res_buf->target_image);

    Local<Object> result = Object::New();
    createResult(
      expect_image, target_image,
      res_buf->flowx, res_buf->flowy,
      threshold, span,
      res_buf->time,
      result);

    // 結果をコールバック
    const unsigned argc = 1;
    Local<Value> argv[argc] = { Local<Value>::New(result) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);

    delete res_buf->flowx;
    delete res_buf->flowy;
    delete res_buf;
  }

  // ConsumerThreadに完了を通知して、終了するまで待つ
  isRunning = false;
  uv_cond_broadcast(&requestNotification);
  for(int i = 0; i < MAX_CONSUMERS; i++) {
    assert(0 == uv_thread_join(&cthreads[i]));
  }

  uv_cond_destroy(&responseNotification);
  uv_cond_destroy(&requestNotification);
  uv_mutex_destroy(&req_mutex);
  uv_mutex_destroy(&res_mutex);
 
  fprintf(stdout, "=== end consumer-producer test ===\n");

  return 0;
}
