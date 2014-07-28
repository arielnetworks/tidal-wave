export PATH=/usr/local/cuda-6.0/bin/:$PATH
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/cuda-6.0/lib64/:$LD_LIBRARY_PATH
. ~/.nvm/nvm.sh 
nvm use v0.10.29
node-gyp configure
node-gyp build

Node.jsのネイティブモジュールで別スレッドで処理したい
http://kimitok.hateblo.jp/entry/2014/04/16/223643

複数回のコールバックを呼びたい場合の参考
https://github.com/mapbox/node-sqlite3/blob/master/src%2Fasync.h

オブジェクトの配列を返す方法
http://stackoverflow.com/questions/23123406/v8-array-of-objects
