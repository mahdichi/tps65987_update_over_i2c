#!/bin/bash

echo compile programm
mkdir -p build
cd build/
cmake ../
make

# sudo ./dvr_cmd


exit 0