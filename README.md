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

## Reference

- "低レイヤを知りたい人のためのCコンパイラ作成入門", https://www.sigbus.info/compilerbook
- and many other websites such as Stack Overflow that I referred to while making this.
