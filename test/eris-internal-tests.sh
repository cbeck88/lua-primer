#!/bin/bash
set -e
cd stage_lua

gdb -return-child-result -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./persist persist.lua test.eris && gdb -return-child-result -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./unpersist unpersist.lua test.eris

cd ..
