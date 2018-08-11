# tinychain_cpp

tinychain ported to C++

original python code is
https://github.com/jamesob/tinychain

## How to begin

```
$ git clone --recursive git@github.com:Kourin1996/tinychain_cpp.git
$ (mkdir -p tinychain_cpp/lib/evpp/build && cd tinychain_cpp/lib/evpp/build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j)
$ (cd tinychain_cpp/lib/cereal && cmake .)
$ cd tinychain_cpp
$ cmake . && make
```
