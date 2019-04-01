#!/bin/bash

apt-get install -y cmake
apt-get install -y build-essential

mkdir -p ./client/main/build
mkdir -p ./server/main/build

mkdir ./tmp

cd tmp

########################################
# oatpp

git clone https://github.com/oatpp/oatpp

cd oatpp
mkdir build && cd $_
cmake -DOATPP_DISABLE_POOL_ALLOCATIONS=ON -DOATPP_DISABLE_ENV_OBJECT_COUNTERS=ON -DOATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT=1 -DCMAKE_BUILD_TYPE=Release ..

make
make install

cd ../../

########################################
# oatpp

git clone https://github.com/oatpp/oatpp-websocket

cd oatpp-websocket
mkdir build && cd $_
cmake ..

make
make install

cd ../../

########################################

cd ../
