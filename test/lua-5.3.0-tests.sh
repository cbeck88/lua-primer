#!/bin/bash
set -e

cd lua-5.3.1-tests && ../stage/lua -e "_U=true" all.lua
