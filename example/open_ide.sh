#!/bin/bash

PATH=~/bin:$PATH

BUILD_CONFIG=Debug
export OS=`uname`
if [ $OS = "Darwin" ]; then
    cmake -E make_directory MacBuild
    pushd MacBuild
    cmake -G "XCode" CMAKE_BUILD_TYPE:STRING=$BUILD_CONFIG -D TARGET_PLATFORM_32:BOOL=OFF -DCMAKE_OSX_SYSROOT=macosx ..
    popd
    cmake --open MacBuild
    exit
fi

cmake -E make_directory build64
pushd build64
cmake -D CMAKE_BUILD_TYPE:STRING=$BUILD_CONFIG -D TARGET_PLATFORM_32:BOOL=OFF ..
popd