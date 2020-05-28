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

assert 0 "0;"
assert 42 "42;"

assert 57 "55+5-3;"

assert 57 "55 + 5 - 3;"
assert 3 " 5 + 5  - 7;"

assert 47 "5 + 6 * 7;"
assert 15 "5 * (9 - 6);"
assert 4 "(3 + 5) / 2;"
assert 4 "(5 + 4) / 2;"

assert 10 "-10 + 20;"
assert 5 "10 + -5;"

assert 1 "10 == 2 + 2 * 4;"
assert 0 "10 != 10;"
assert 1 "10 > 5;"
assert 0 "2 * 5 >= 11;"
assert 0 "10 <= 5;"
assert 1 "5 < 2 * 3;"

assert 10 "a = 10; a;"
assert 10 "b = 5; b + 5;"
assert 32 "a = 4; b = 4 + a; a * b;"
assert 6 "foo = 1; bar = 2 + 3; foo + bar;"

assert 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
assert 5 "return 5; return 8;"

assert 1 "if (1 == 1) return 1; else return 2;"
assert 5 "i = 5; if (i == 4) return 3; return 5;"

assert 5 "i = 0; for (; i < 5; ) i = i + 1; return i;"
assert 16 "ans = 1; for (i = 0; i < 4; i = i + 1) ans = ans * 2; return ans;"

assert 17 "test = 20; while (test / 3 != 5) test = test - 1; return test;"

echo "OK"
