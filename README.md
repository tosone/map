# map

Some useful tools and algorithms playground.

## Prerequisites

Make sure you have installed all of the following prerequisites on your development machine:

- gcc or clang - Compile the project, on MacOS you should install clang with `xcode-select --install`.
- make - Build automation tool that automatically builds executable programs and libraries from source code by reading files called Makefiles which specify how to derive the target program.

### MacOS

``` sh
xcode-select --install
brew install make
```

### Ubuntu

``` bash
apt-get install build-essential make
```

## QA

- How to convert the dot file to png?

  You should install graphviz first, then convert filename.dot to filename.png with `dot -Tpng filename.dot -o filename.png`。
