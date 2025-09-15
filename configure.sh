#!/bin/bash

# RXT_SOURCE_PATH=
LIBCPP_PATH=/opt/homebrew/opt/llvm/lib/c++

ROOTDIR=.
BUILDDIR=$ROOTDIR/build

options=
if [[ -n "$RXT_SOURCE_PATH" ]]; then
    options+=" -D RXT_AS_SUBDIR=$(readlink -f $RXT_SOURCE_PATH)"
fi
if [[ -n "$LIBCPP_PATH" ]]; then
    options+=" -D CMAKE_EXE_LINKER_FLAGS=-L$LIBCPP_PATH -Wl,-rpath,$LIBCPP_PATH"
fi

set -x

cmake -S "$ROOTDIR" -B "$BUILDDIR" $options
