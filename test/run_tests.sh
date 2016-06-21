#!/bin/bash

set -e

b2 "$@"
cd stage && gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./core
cd ..

# b2 clean
# b2 cxxflags="-std=c++11 -DLUA_32BITS"
# echo
# cd stage && gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./core
# cd ..
