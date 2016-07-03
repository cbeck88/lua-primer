#!/bin/bash

set -e

b2 "$@"
cd stage && gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./core
cd ..

cd stage && gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./visitable
cd ..

cd stage && gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./std
cd ..

if [ -e stage/boost ]
then
  cd stage && gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./boost
  cd ..
fi

if [ -e stage/api ]
then
  cd stage && gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./api
  cd ..
fi
