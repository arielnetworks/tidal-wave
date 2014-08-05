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

#include "main.h"

// CPUのコア数とGPUのデバイス数の合計に設定すると最もパフォーマンスがよい
#define MAX_CONSUMERS 5

using namespace v8;
using namespace std;
 
bool listFiles(const string filepath, int curdepth, vector<string> &list);

typedef struct {
  ngx_queue_t queue;
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

  OpticalFlow *opt;
  double threshold;
  int span;
} ResponseBuffer;

typedef struct {
  string expect_path;
  string target_path;
  OpticalFlow *opt;
  double threshold;
  int span;
} ProducerArg;

typedef struct {
  int num;
} ConsumerArg;

static ngx_queue_t req_queue;
static ngx_queue_t res_queue;

static uv_async_t async;

static uv_mutex_t req_mutex;
static uv_mutex_t res_mutex;
static uv_cond_t responseNotification;
static uv_cond_t requestNotification;

static uv_mutex_t msg_mutex;
static std::vector<ResponseBuffer*> *msg_buffer;

static volatile int callback_count = 0;
static volatile bool isRunning = true;

float calcOpticalFlow(string pathL, string pathR, bool gpuMode, cv::Mat &flowx, cv::Mat &flowy);

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
  delete carg;
}


// 指定された2つのディレクトリの中からパスが一致する画像の組を作って、画像比較処理の依頼をおこなう
void producerThread(uv_work_t* req) {


  // ConsumerThreadの生成
  uv_thread_t cthreads[MAX_CONSUMERS];
  for(int i = 0; i < MAX_CONSUMERS; i++) {
    ConsumerArg *carg = new ConsumerArg();
    carg->num = i;
    assert(0 == uv_thread_create(&cthreads[i], consumerThread, (void*)carg));
  }


  ProducerArg *parg = (ProducerArg*)req->data;

  vector<string> expect_files;
  vector<string> target_files;

  listFiles(parg->expect_path, 1, expect_files);
  listFiles(parg->target_path, 1, target_files);

  int request_count = 0;

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
    res_buf->opt = parg->opt;
    res_buf->threshold = parg->threshold;
    res_buf->span = parg->span;

    // sendCallback向けにデータを詰める
    uv_mutex_lock(&msg_mutex);
    msg_buffer->push_back(res_buf);
    uv_mutex_unlock(&msg_mutex);

    // sendCallback処理を実行してもらうように通知する
    uv_async_send(&async);
  }

  // ConsumerThreadに完了を通知して、終了するまで待つ
  isRunning = false;
  uv_cond_broadcast(&requestNotification);
  for(int i = 0; i < MAX_CONSUMERS; i++) {
    assert(0 == uv_thread_join(&cthreads[i]));
  }

  // すべてのsendCallback処理が完了するまで待つ。
  // producerThreadが完了するとすぐにafterが呼ばれて、mutexとかいろいろ解放されてしまうので。
  while (true) {
    uv_mutex_lock(&msg_mutex);
    if (request_count == callback_count) {
      uv_mutex_unlock(&msg_mutex);
      break;
    } 
    uv_mutex_unlock(&msg_mutex);
    sleep(1);
  }
  //delete req;
  cout << "finish producer" << endl;
}

// Node.js側のコールバック関数を呼び出してオプティカルフローの計算結果を通知する
// uv_async_sendを複数回呼んでもsendCallbackは1回にまとめられることがあるので、msg_bufferで複数のデータをやりとり。
void sendCallback(uv_async_t *handle, int status) {
  HandleScope scope;

  uv_mutex_lock(&msg_mutex);
  for(vector<ResponseBuffer*>::iterator it = msg_buffer->begin(); it != msg_buffer->end(); it++) {
    ResponseBuffer *res_buf = *it;

    string expect_image = string(res_buf->expect_image);
    string target_image = string(res_buf->target_image);

    res_buf->opt->Emit(
      expect_image, target_image, 
      res_buf->flowx, res_buf->flowy, 
      res_buf->threshold, res_buf->span, 
      res_buf->time);

    delete res_buf->flowx;
    delete res_buf->flowy;
    delete res_buf;
    callback_count++;
  }
  msg_buffer->clear();
  uv_mutex_unlock(&msg_mutex);

};

// すべての処理が完了した時に呼び出される
void after(uv_work_t* req, int status) {

  // Node.js側に終了のコールバック
  ProducerArg *parg = (ProducerArg*)req->data;
  parg->opt->Finish();

  uv_cond_destroy(&responseNotification);
  uv_cond_destroy(&requestNotification);
  uv_mutex_destroy(&req_mutex);
  uv_mutex_destroy(&res_mutex);
  uv_mutex_destroy(&msg_mutex);
  uv_close((uv_handle_t*) &async, NULL);
  delete msg_buffer;
}


 
int dispatch(
  const string expect_path,
  const string target_path,
  const double threshold,
  const int span, 
  OpticalFlow *opt
){
  cout << "device count: " << cv::gpu::getCudaEnabledDeviceCount() << endl;
  fprintf(stdout, "=== start consumer-producer test ===\n");

  ngx_queue_init(&req_queue);
  ngx_queue_init(&res_queue);
  assert(0 == uv_mutex_init(&req_mutex));
  assert(0 == uv_mutex_init(&res_mutex));
  assert(0 == uv_mutex_init(&msg_mutex));
  assert(0 == uv_cond_init(&responseNotification));
  assert(0 == uv_cond_init(&requestNotification));
  msg_buffer = new vector<ResponseBuffer*>();

  callback_count = 0;
  isRunning = true;


  // ProducerThreadの生成
  ProducerArg *parg = new ProducerArg();
  parg->expect_path = expect_path;
  parg->target_path = target_path;
  parg->opt = opt;
  parg->threshold = threshold;
  parg->span = span;

  uv_work_t *req = new uv_work_t();
  req->data = parg;
  uv_async_init(uv_default_loop(), &async, sendCallback);
  uv_queue_work(uv_default_loop(), req, producerThread, after);

 
  fprintf(stdout, "=== end consumer-producer test ===\n");

  return 0;
}
