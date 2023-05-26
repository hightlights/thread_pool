#!/bin/bash

cur_dir=$PWD

function build_help(){
    echo "build.sh need args to specify build action."
    echo ""
    echo "./build.sh --b"
    echo "build the target directly, if this is the first build, --r should be used"
    echo ""
    echo "./build.sh --r"
    echo "rebuild Makefile and then build target"
    echo ""
    echo "./build.sh --c"
    echo "remove the cmake cache files"
    echo ""
    echo "you can also use --rc/cr to remove cache files and then rebuild."
}

function target_link(){
   if [ ! -f "threadpool" ]; then
        ln -s $cur_dir/build/bin/threadpool threadpool
    fi 
}

function mkdir_build(){
    if [ ! -d "build" ];then
        mkdir build
        mkdir ./build/bin
        ln -s $cur_dir/CMakeLists.txt ./build/CMakeLists.txt
    fi
}

function building(){
    mkdir_build
    cd build
    cmake .
    make
    cd ..
}

# main
case $1 in
"--b")
    cd build
    make
    ;;
"--r")
    building
    target_link
    ;;
"--c")
    rm -rf build
    rm threadpool
    ;;
"--rc"|"--cr")
    rm -rf build
    building
    target_link
    ;;
*)
    build_help
    ;;
esac


