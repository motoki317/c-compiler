#!/bin/bash

set -eu

run_single() {
  NAME="$1"
  TIME="$(time ( ./tmp ) 2>&1 1>/dev/null )"
  echo "$NAME time: $TIME"
}

run_test() {
  FILE="$1"
  echo "Running test on $FILE ..."
  echo "----"

  # gcc
  cc -o tmp "test/$FILE"
  run_single gcc

  echo "----"

  # this compiler
  ./main "test/$FILE" > tmp.s
  cc -o tmp tmp.s
  run_single my-compiler

  echo "----"
}

run_test "simple_loop.c"
# cannot run the same file because the compiler hasn't supported preprocessors yet
#run_test "sudoku_solver.c"

echo "Tests bench OK"
