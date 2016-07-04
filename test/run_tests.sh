#!/bin/bash

set -e

b2 "$@"

for file in stage/*
do
  if [[ ${file} != "stage/lua" && ${file} != "stage/persist" && ${file} != "stage/unpersist" && ${file} != "stage/persist.lua" && ${file} != "stage/unpersist.lua" ]]
  then
    echo ${file} "..."
    gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./${file}
  fi
done
