#!/bin/bash

set -x

# Get the tool
git clone https://github.com/MartinVerges/mklittlefs.git tmp

# Build
cd tmp/
git submodule update --init
make dist
cp mklittlefs ../
cd ..

# Cleanup
rm -rf tmp
