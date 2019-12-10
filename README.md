# JFTTCompiler 2018 (2019)

## Requirements 

* CMake 3.7+
* clang/clang++
* bison 3.2.2
* flex 2.5.35

## How to run

Create `build` and `install` directory:
```
> mkdir build
> mkdir install
```

Change directory to `build`:
```
cd build
```

Then run:
```
cmake .. -DCMAKE_INSTALL_PREFIX=../install -DDISABLE_TESTS=1
make install
```

Executable should be in `install/bin/compiler`

## Troubleshooting

- I cannot compile using gcc.

Project was developed with clang llvm compiler in mind. To force CMake into using clang:
```
> export CC=/usr/bin/clang
> export CXX=/usr/bin/clang++
> cmake .. -DCMAKE_INS... # steps from above
```

- Cannot compile on macOS (bison error)

macOS ships with very old version of bison (2.3 on Mojave). The project requires at least version 3.2.2. 
You can also specify `-DBISON_PATH`, if you have bison 3.2.2+ executable installed in different location (i.e. from Homebrew).

## Tests

Project is using Google Test 1.8.1 as unit testing framework. To enable tests, set `DISABLE_TESTS` to 0 when running cmake.

## Automated tests using VM

Automated tests can be done using `tools/autoTest.py`.
