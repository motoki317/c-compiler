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

assert 0 "int main() { 0; }"
assert 42 "int main() { 42; }"

assert 57 "int main() { 55+5-3; }"

assert 57 "int main() { 55 + 5 - 3; }"
assert 3 "int main() {  5 + 5  - 7; }"

assert 47 "int main() { 5 + 6 * 7; }"
assert 15 "int main() { 5 * (9 - 6); }"
assert 4 "int main() { (3 + 5) / 2; }"
assert 4 "int main() { (5 + 4) / 2; }"

assert 10 "int main() { -10 + 20; }"
assert 5 "int main() { 10 + -5; }"

assert 1 "int main() { 10 == 2 + 2 * 4; }"
assert 0 "int main() { 10 != 10; }"
assert 1 "int main() { 10 > 5; }"
assert 0 "int main() { 2 * 5 >= 11; }"
assert 0 "int main() { 10 <= 5; }"
assert 1 "int main() { 5 < 2 * 3; }"

assert 10 "int main() { int a; a = 10; a; }"
assert 10 "int main() { int b; b = 5; b + 5; }"
assert 32 "int main() { int a; a = 4; int b; b = 4 + a; a * b; }"
assert 6 "int main() { int foo; foo = 1; int bar; bar = 2 + 3; foo + bar; }"

assert 14 "int main() { int a; a = 3; int b; b = 5 * 6 - 8; return a + b / 2; }"
assert 5 "int main() { return 5; return 8; }"

assert 1 "int main() { if (1 == 1) return 1; else return 2; }"
assert 5 "int main() { int i; i = 5; if (i == 4) return 3; return 5; }"

assert 5 "int main() { int i; i = 0; for (; i < 5; ) i = i + 1; return i; }"
assert 16 "int main() { int ans; ans = 1; int i; for (i = 0; i < 4; i = i + 1) ans = ans * 2; return ans; }"

assert 17 "int main() { int test; test = 20; while (test / 3 != 5) test = test - 1; return test; }"

assert 2 "int main() { int i; i = 2; if (i == 3) { i = i + 1; i = i * 2 + 1; } return i; }"
assert 7 "int main() { int i; i = 2; if (i == 2) { i = i + 1; i = i * 2 + 1; } return i; }"

assert 13 "int fib(int x) { if (x <= 1) return x; return fib(x - 1) + fib(x - 2); } int main() { return fib(7); }"
assert 10 "int tarai(int x, int y, int z) { if (x <= y) return y; return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y)); } int main() { return tarai(10, 6, 0); }"

assert 3 "int main() { int x; x = 3; int *y; y = &x; return *y; }"

assert 3 "int main() { int x; int *y; y = &x; *y = 3; return x; }"

assert 4 "int main() { int x; return sizeof(x); }"
assert 8 "int main() { int *y; return sizeof(y); }"
assert 4 "int main() { int x; return sizeof(x + 3); }"
assert 8 "int main() { int *y; return sizeof(y + 3); }"
assert 4 "int main() { int *y; return sizeof(*y); }"
assert 4 "int main() { return sizeof(1); }"
assert 4 "int main() { return sizeof(sizeof(1)); }"

assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"
assert 3 "int main() { int a[2]; *(a + 1) = 2; *a = 1; int *p; p = a; return *p + *(p + 1); }"

assert 3 "int main() { int a[2]; a[0] = 1; a[1] = 2; int *p; p = a; return a[0] + a[1]; }"

assert 57 "int x; int main() { x = 57; return x; }"
assert 110 "int a; int main() { int *p; p = &a; *p = 110; return *p; }"
assert 8 "int a[2]; int main() { int x; x = 5; a[0] = 1; a[1] = 2; int *p; p = a; return p[0] + a[1] + x; }"

assert 18 "int y[3]; int main() { int x; x = 5; int i; for (i = 0; i < 3; i = i + 1) { y[i] = i * 3 + 3; } if (x == 5) { return y[0] + y[1] + y[2]; } else { return y[0] + y[1]; } }"

echo "OK"
