# c-compiler

This is a mini C compiler that compiles a subset of C programming language.

## Development

Docker used for development and testing:
- `make buildImage` to build the Docker image for development and testing
- `make docker` to launch the container
    - Inside the container, `make main` to build the compiler
    - `make test` to run tests

Actual source code is inside the `/compiler` directory.
Test source code and sample source codes that can be compiled by this compiler are inside the `/compiler/test` directory.

## Tests

Tests located in the `/compiler/test` directory

### Run test

`make test` to run the `/compiler/test.sh` file to
compile `/compiler/test/main.c` and run it.

### Running time comparison

`make bench` to run the `/compiler/test_bench.sh` file.

Example output

`/compiler/test/simple_loop.c`:
```text
Running test on simple_loop.c ...
----
gcc time: 
real    0m2.537s
user    0m2.530s
sys     0m0.003s
----
my-compiler time: 
real    0m6.495s
user    0m6.468s
sys     0m0.005s
----
```

## Reference

- "低レイヤを知りたい人のためのCコンパイラ作成入門", https://www.sigbus.info/compilerbook
- and many other websites such as Stack Overflow that I referred to while making this.
