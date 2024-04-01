# largs, the line oriented version of xargs

## Motivation

I always want to open the latest file I just downloaded, so I open the terminal
and type

```sh
ls -t ~/Downloads | head -n1

# Some long file name containing spaces
```

and I want to pipe that to the `open` command. The `open` command on MacOS does
not support reading from stdin, so I use `xargs`. But I can't just pipe to
`xargs`. I have to do this instead

```sh
ls -t ~/Downloads | head -n1 | tr '\n' '\0' | xargs -0 open
```

which sucks.

So I wrote `largs`.

## Usage

```
usage: ./largs [-ch] [-j replstr] [utility [argument ...]]
```

It doesn't do a lot of stuff xargs does. But it is enough for my use case.

- `-c`
  Put all input lines into one line of arguments

  ```sh
  ls | largs rm

  # rm file1 file2 file3...
  ```

- `-h`
  Prints help message and exits immediately

- `-j replstr`
  Same as `-J` in xargs. Defaults to `%`

## Compile

```
clang++ -std=c++20 -O2 -o largs largs.cpp
```

or 

```
g++ -std=c++20 -O2 -o largs largs.cpp
```

or compile with your favourite C++ compiler and specify it to use C++ 20.

You may want to place `largs` to your $PATH. Maybe `/usr/local/bin/`
