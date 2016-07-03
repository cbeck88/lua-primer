#!/bin/bash
set -e
cd stage

./persist persist.lua test.eris && ./unpersist unpersist.lua test.eris

cd ..
