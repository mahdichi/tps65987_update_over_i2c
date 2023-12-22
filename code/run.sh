#!/bin/bash

echo compile programm
mkdir -p build
cd build/
cmake ../
make

sudo ./tps_flasher


exit 0