export PATH=/usr/local/cuda-6.0/bin/:$PATH
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/cuda-6.0/lib64/:$LD_LIBRARY_PATH
. ~/.nvm/nvm.sh 
nvm use v0.10.29
node-gyp configure
node-gyp build

