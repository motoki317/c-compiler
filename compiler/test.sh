#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./main "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "main() { 0; }"
assert 42 "main() { 42; }"

assert 57 "main() { 55+5-3; }"

assert 57 "main() { 55 + 5 - 3; }"
assert 3 "main() {  5 + 5  - 7; }"

assert 47 "main() { 5 + 6 * 7; }"
assert 15 "main() { 5 * (9 - 6); }"
assert 4 "main() { (3 + 5) / 2; }"
assert 4 "main() { (5 + 4) / 2; }"

assert 10 "main() { -10 + 20; }"
assert 5 "main() { 10 + -5; }"

assert 1 "main() { 10 == 2 + 2 * 4; }"
assert 0 "main() { 10 != 10; }"
assert 1 "main() { 10 > 5; }"
assert 0 "main() { 2 * 5 >= 11; }"
assert 0 "main() { 10 <= 5; }"
assert 1 "main() { 5 < 2 * 3; }"

assert 10 "main() { a = 10; a; }"
assert 10 "main() { b = 5; b + 5; }"
assert 32 "main() { a = 4; b = 4 + a; a * b; }"
assert 6 "main() { foo = 1; bar = 2 + 3; foo + bar; }"

assert 14 "main() { a = 3; b = 5 * 6 - 8; return a + b / 2; }"
assert 5 "main() { return 5; return 8; }"

assert 1 "main() { if (1 == 1) return 1; else return 2; }"
assert 5 "main() { i = 5; if (i == 4) return 3; return 5; }"

assert 5 "main() { i = 0; for (; i < 5; ) i = i + 1; return i; }"
assert 16 "main() { ans = 1; for (i = 0; i < 4; i = i + 1) ans = ans * 2; return ans; }"

assert 17 "main() { test = 20; while (test / 3 != 5) test = test - 1; return test; }"

assert 2 "main() { i = 2; if (i == 3) { i = i + 1; i = i * 2 + 1; } return i; }"
assert 7 "main() { i = 2; if (i == 2) { i = i + 1; i = i * 2 + 1; } return i; }"

assert 13 "fib(x) { if (x <= 1) return x; return fib(x - 1) + fib(x - 2); } main() { return fib(7); }"
assert 10 "tarai(x, y, z) { if (x <= y) return y; return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y)); } main() { return tarai(10, 6, 0); }"

assert 3 "main() { x = 3; y = &x; return *y; }"

echo "OK"
