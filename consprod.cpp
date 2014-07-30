#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include <iostream>
#include <vector>
#include <sstream>
 
#include "uv.h"
 
#define MAX_CONSUMERS 3
#define MAX_LOOPS 1

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
  char expect_path[255];
  char target_path[255];
} response_buffer;
 
static ngx_queue_t req_queue;
static ngx_queue_t res_queue;

static uv_mutex_t req_mutex;
static uv_mutex_t res_mutex;
static uv_cond_t empty;
static uv_cond_t full;

static volatile int finished_consumers = 0;
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
    string data = string(buf->expect_path);
    free(buf);

    uv_cond_signal(&empty);
    uv_mutex_unlock(&req_mutex);


    
    uv_mutex_lock(&res_mutex);
    response_buffer* res_buf;
    res_buf = (response_buffer*)malloc(sizeof(response_buffer));
    ngx_queue_init(&res_buf->queue);
    ngx_queue_insert_tail(&req_queue, &res_buf->queue);
    uv_mutex_unlock(&res_mutex);

    finished_requests++;
  }
  cout << "finish consumer"  << carg->num << endl;
}
 
int consprod() {
  int i;
  uv_thread_t cthreads[MAX_CONSUMERS];
  uv_thread_t pthread;
 
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
  parg.expect_path = string("./public/images");
  parg.target_path = string("./public/images");

  assert(0 == uv_thread_create(&pthread, producer, (void*)&parg));
  assert(0 == uv_thread_join(&pthread));

  while(!ngx_queue_empty(&res_queue)) {
    uv_cond_wait(&empty, &res_mutex);
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
  cout << "request number : " << request_number << endl;
 
  fprintf(stdout, "=== end consumer-producer test ===\n");
  return 0;
}
