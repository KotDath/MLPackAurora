#!/bin/bash

pushd $1/_deps/openblas-src
/usr/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON -DDYNAMIC_ARCH=ON -DTARGET=$2 -DBINARY=$3 .
make -j$(nproc)
popd
