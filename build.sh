#!/bin/sh

opts="-Wno-c++11-compat-deprecated-writable-strings -Wno-write-strings -g"
code=$PWD
cd $code/build
g++ $opts $code/pinky_crt_base.cpp -o pinky
cd $code


