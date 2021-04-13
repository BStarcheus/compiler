#!/bin/sh
./compiler "$@"
clang out.s runtime.o -o a.out -lm