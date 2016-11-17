#!/bin/bash
set -e

cd lua-5.3.3-tests && ../stage_lua/lua -e "_port=true" all.lua
