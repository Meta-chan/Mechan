#!/bin/sh
TD_DIR=~/Mechan/td
IR_DIR=~/ir
#ARCH=-A x64

if [ ! -d build ]; then
	mkdir build
	cd build
	cmake ${ARCH} -DCMAKE_BUILD_TYPE=Release -DTd_DIR=${TD_DIR}/lib/cmake/Td -DIr_DIR=${IR_DIR} ..
	cd ..
fi

cmake --build build --config Release
