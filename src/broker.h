#ifndef BROKER_H
#define BROKER_H

#include <iostream>
#include <vector>
#include <sstream>

#include <node.h>
#include <v8.h>

#include "opencv2/core/core.hpp"
#include "observer.h"
#include "manager.h"

namespace tidalwave {

  /*
   * @brief Node.jsとC++の世界を仲介するクラス
   */
  class Broker : public node::ObjectWrap, public Observer<Response> {
  public:

    /*
     * @brief Node.jsからC++を呼び出すための情報をセットアップ
     * @param target Node.jsの世界からやってきたオブジェクト
     */
    static void initialize(v8::Handle<v8::Object> &target);

    /*
     * @brief C++からNode.jsに処理結果の通知をおこなう
     * @param value 処理結果
     */
    virtual void onNext(const Response &value);

    /*
     * @brief C++からNode.jsにエラーの通知をおこなう
     * @param reason エラー情報
     */
    virtual void onError(const std::string &reason);

    /*
     * @brief C++からNode.jsにすべての処理が完了したことを通知する
     */
    virtual void onCompleted();

  private:
    Broker();

    virtual ~Broker();

    /*
     * @brief このクラスのインスタンスを生成する
     */
    static v8::Handle<v8::Value> createInstance(const v8::Arguments &args);

    /*
     * @brief Node.jsからのリクエストをさばく
     */
    static v8::Handle<v8::Value> requestFromClient(const v8::Arguments &args);

    /*
     * @brief C++側の処理結果をNode.jsで扱える形式に変換する
     */
    static v8::Local<v8::Object> convertResult(const Response &value);

    static int getInt32OrDefault(const v8::Local<v8::Value> &arg, const std::string& name, int defaultValue);
    static double getNumberOrDefault(const v8::Local<v8::Value> &arg, const std::string& name, double defaultValue);

    // この2つのインスタンスはプログラムが終了するまで解放しない
    static Broker *broker;
    static Manager *dispatcher;

  };
}

#endif // BROKER_H
