# JFTTCompiler 2018 (2019)

Compiler of simple pascal-like language, written for 2018-2019 edition of "Formal Languages and Translation Techniques" university course.

## Syntax

```
program      -> DECLARE declarations IN commands END

declarations -> declarations pidentifier;
            | declarations pidentifier(num:num);
            |

commands     -> commands command
            | command

command      -> identifier := expression;
            | IF condition THEN commands ELSE commands ENDIF
            | IF condition THEN commands ENDIF
            | WHILE condition DO commands ENDWHILE
            | DO commands WHILE condition ENDDO
            | FOR pidentifier FROM value TO value DO commands ENDFOR
            | FOR pidentifier FROM value DOWNTO value DO commands ENDFOR
            | READ identifier;
            | WRITE value;

expression   -> value
            | value + value
            | value - value
            | value * value
            | value / value
            | value % value

condition    -> value = value
            | value != value
            | value < value
            | value > value
            | value <= value
            | value >= value

value        -> num
            | identifier

identifier   -> pidentifier
            | pidentifier(pidentifier)
            | pidentifier(num)
```

Example programs can be found in `resources/` dir.

## VM

Binaries for VM created by dr Maciej Gębala ([course website](https://cs.pwr.edu.pl/gebala/dyd/jftt2019.html)), can be found in `vm/` dir. Source code is available [here](https://cs.pwr.edu.pl/gebala/dyd/jftt2019/labor4.zip).

## Requirements

* CMake 3.7+
* clang/clang++
* bison 3.2.2+
* flex 2.5.35+

## How to build

Create `build` and `install` directory:
```
> mkdir build
> mkdir install
```

Change directory to `build`:
```
> cd build
```

Then run:
```
> cmake .. -DCMAKE_INSTALL_PREFIX=../install -DDISABLE_TESTS=1
> make install
```

Executable should be in `install/bin/compiler`

## Usage:

`compiler [OPTIONS] inputFile outputFile`

Available options:
- `m` print memory map to stderr
- `i` print low-level immediate representation to stderr
- `s` print simplified low-level immediate representation to stderr

## Troubleshooting

- Cannot compile using gcc.

Project was developed with clang llvm compiler in mind. To force CMake into using clang:
```
> export CC=/usr/bin/clang
> export CXX=/usr/bin/clang++
> cmake .. -DCMAKE_INS... # steps from above
```

- Cannot compile on macOS (bison error)

macOS ships with very old version of bison (2.3 on Mojave). The project requires at least version 3.2.2.
You can also specify `-DBISON_PATH`, if you have bison 3.2.2+ executable installed in custom location (i.e. from Homebrew).

## Tests

Project is using Google Test 1.8.1 as unit testing framework. To enable tests, set `DISABLE_TESTS` to 0 when running cmake.

## Automated tests using VM

Automated tests can be done using `tools/autoTest.py`.
