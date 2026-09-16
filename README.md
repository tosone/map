# map

Some useful tools and algorithms playground.

## Prerequisites

Make sure you have installed all of the following prerequisites on your development machine:

- gcc or clang - Compile the project, on MacOS you should install clang with `xcode-select --install`.
- g++ or clang++ (C++17) - Compile `src/hash.cpp` and `src/base64.cpp`, which wrap the header-only hashlib library in `include/hashlib` and `include/base64.hpp`. They come with `build-essential` on Ubuntu and with the Xcode command line tools on MacOS.
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
