#!/bin/bash

set -eux

./main ./test/main.c > tmp.s
cc -o tmp tmp.s
./tmp

echo "OK"
