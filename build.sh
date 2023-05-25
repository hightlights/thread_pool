#!/bin/bash

cur_dir=$PWD

function build_help(){
    echo "build.sh need args to specify build action."
    echo ""
    echo "./build.sh --b"
    echo "build the target directly"
    echo ""
    echo "./build.sh --r"
    echo "rebuild Makefile and then build target"
    echo ""
    echo "./build.sh --c"
    echo "remove the cmake cache files"
}

function target_link(){
   if [ ! -f "threadpool" ]; then
        ln -s $cur_dir/build/bin/threadpool threadpool
    fi 
}

# main
if [ ! -d "build" ];then
    mkdir build
    mkdir ./build/bin
    ln -s $cur_dir/CMakeLists.txt ./build/CMakeLists.txt
fi

cd build
if [ $# -eq 0 ]; then
	build_help
elif [ $1 = "--b" ]; then
    make
elif [ $1 = "--r" ]; then
    cmake .
    make
    cd ..
    target_link
elif [ $1 = "--c" ]; then
    cd ..
    rm -rf build
fi


