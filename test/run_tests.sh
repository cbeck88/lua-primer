#!/bin/bash

set -e

bjam clean
bjam cxxflags="-std=c++11"
echo
cd stage && gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./core
cd ..

# bjam clean
# bjam cxxflags="-std=c++11 -DLUA_32BITS"
# echo
# cd stage && gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./core
# cd ..
