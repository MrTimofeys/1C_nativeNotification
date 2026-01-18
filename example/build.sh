#!/bin/bash

#build script

PATH=~/bin:$PATH
set -e

ZIP="$1"
[ ! -z "$ZIP" ] && FROM_ZIP="-DFROM_ZIP=$ZIP"

ARCH_NAME=example.zip
ARCH_OSX=mac.pkg.zip
ARCH_LIN=lin.pkg.zip
ARCH_WIN=win.pkg.zip
ARCH_OSXARM=mac.arm64.pkg.zip
ARCH_LINARM=lin.aarch64.pkg.zip
ARCH_LINE2K=lin.e2kv4.pkg.zip

cmake -E remove_directory ./bin
cmake -E remove_directory ./pkg
cmake -E remove_directory ./debug_info
cmake -E remove_directory ./build64
cmake -E remove_directory ./build32
export OS=`uname`
if [ $OS = "Darwin" ]; then
    cmake -E remove_directory MacBuild
    cmake -E make_directory MacBuild
    cmake -E make_directory bin
    export OS=`uname -m`
    if [ "$OS" = "arm64" ]; then
        ARCH=$ARCH_OSXARM
        SUFFIX=.arm64
    else
        ARCH=$ARCH_OSX
    fi
    pushd MacBuild
    cmake -D CMAKE_BUILD_TYPE:STRING=Release -D TARGET_PLATFORM_32:BOOL=OFF $FROM_ZIP .. 
    cmake --build .
    popd
    cmake -E remove_directory MacBuild
    pushd pkg
    ls -A > ../flist.lst
    cmake -E tar cf ../$ARCH --format=zip --files-from=../flist.lst
    cmake -E remove -f ../flist.lst
    popd

    cmake -E tar cjf mac$SUFFIX.bin.tar.bz2 debug_info/

    if [[ -f $ARCH_LIN && -f $ARCH_WIN && -f $ARCH_LINARM && -f $ARCH_LINE2K ]]; then
        cmake -E echo Make zip-archive
        pushd pkg
        cmake -E tar xf ../$ARCH_WIN
        cmake -E tar xf ../$ARCH_LIN
        cmake -E tar xf ../$ARCH_LINARM
        cmake -E tar xf ../$ARCH_LINE2K
        ls -A > ../flist.lst
        cmake -E tar cf ../$ARCH_NAME --format=zip --files-from=../flist.lst
        popd
        cmake -E remove -f flist.lst
        cmake -E remove $ARCH_WIN
        cmake -E remove $ARCH_LIN
        cmake -E remove $ARCH_LINARM
        cmake -E remove $ARCH_LINE2K
    fi
    exit
fi

export OS=`uname -m`
if [ "$OS" = "aarch64" ]; then
    ARCH=$ARCH_LINARM
    SUFFIX=.aarch64
elif [ "$OS" = "e2k" ]; then
    ARCH=$ARCH_LINE2K
    SUFFIX=.e2kv4
else
    ARCH=$ARCH_LIN
fi

cmake -E echo "Build 64"
cmake -E remove_directory build64
cmake -E make_directory build64
cd build64 ;
cmake -D CMAKE_BUILD_TYPE:STRING=Release -D TARGET_PLATFORM_32:BOOL=OFF $FROM_ZIP .. ;
cmake --build . -- -j 2
cd ..
cmake -E remove_directory build64

if [ "$OS" != "aarch64" -a "$OS" != "e2k" ]; then
    cmake -E echo "Build 32"
    cmake -E remove_directory build32
    cmake -E make_directory build32
    cd build32
    cmake -D CMAKE_BUILD_TYPE:STRING=Release -D TARGET_PLATFORM_32:BOOL=ON $FROM_ZIP .. ;
    cmake --build . -- -j 2
    cd ..
    cmake -E remove_directory build32
fi

cd pkg
ls -A > ../flist.lst
cmake -E tar cf ../$ARCH --format=zip --files-from=../flist.lst
cmake -E remove -f ../flist.lst
cd ..

cmake -E tar cjf lin$SUFFIX.bin.tar.bz2 debug_info/

if [[ -f $ARCH_OSX && -f $ARCH_WIN && -f $ARCH_OSXARM]]; then
    cmake -E echo Make zip-archive
    cd pkg
    cmake -E tar xf ../$ARCH_WIN
    cmake -E tar xf ../$ARCH_OSX
    cmake -E tar xf ../$ARCH_OSXARM
    ls -A > ../flist.lst
    cmake -E tar cf ../$ARCH_NAME --format=zip --files-from=../flist.lst
    cd ..
    cmake -E remove -f flist.lst
    cmake -E remove $ARCH_WIN
    cmake -E remove $ARCH_OSX
    cmake -E remove $ARCH_OSXARM
fi
