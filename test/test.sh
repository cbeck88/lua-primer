#!/bin/bash

set -e

rm -rf bin
rm -rf stage
rm -rf stage_lua
./run_tests.sh $@
