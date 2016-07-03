#!/bin/bash
set -e

wget -nv -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository -y 'deb http://llvm.org/apt/trusty llvm-toolchain-trusty-3.7 main'
sudo apt-get update -qq
sudo apt-get install -qq clang-3.7
