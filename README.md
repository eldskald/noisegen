# noisegen

This project is me practicing more low level programming with raylib. This is kinda like a port of my [Unity Support Textures Generator](https://github.com/eldskald/unity-support-textures-generators) project, this time as a standalone app instead of a menu inside Unity, made with C and raylib.

## Dependencies

This project is using the GNU Toolchain for C/C++ development, in particular, the [GNU Compiler Collection](https://www.gnu.org/software/gcc), the [MinGW-w64](https://www.mingw-w64.org/) compilers and [make](https://www.gnu.org/software/make). If you want to make an HTML5 build, you need to install [emsdk](https://emscripten.org/docs/getting_started/downloads.html). See more details on below.

You'll also need basic libraries used by [raylib](https://github.com/raysan5/raylib) to do graphics and audio. More info on their [wiki](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux).

It's also using [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to format the code, as well as [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) to lint.

## Getting started

After cloning the repository, setup your `.env` file by copying the [.env.example](.env.example) and renaming it to `.env`, and then setting the values. The default values are for working on Linux. If you are working on Windows, use WSL.

Then, run the following to install dependencies:

```console
bin/install-dependencies
```

This will install [raylib](https://github.com/raysan5/raylib) and [raygui](https://github.com/raysan5/raygui). You need [emsdk](https://emscripten.org/docs/getting_started/downloads.html) and its path on `.env` to compile it for web. If you don't have it, it will still work for the desktop versions. More info on the web version later. We are also using [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) and [clang-format](https://clang.llvm.org/docs/ClangFormat.html) for linting and formatting.

To erase dependencies and builds in case you want to redo everything, run:

```console
make clean
```

## Running and building

To make a build, run one of these commands:

```console
make linux   # Makes a Linux build
make windows # Makes a Windows build
make webgl   # Makes a WebGL (HTML5) build
make         # All of the above
```

This will create `linux.zip`, `windows.zip` and `webgl.zip` that can be sent to anyone to unzip and run the game.

To build and run a development build, run the following:

```console
bin/dev
```

It will make a build, run it and delete it after you close it. Easy for quickly compiling changes and running the game with them.

You can also make a development build that won't be ran automatically or deleted after you closing by running this:

```console
make dev
make web-dev
```

This will put the build at the project root. Can be ran on a debugger and can be sent to testers to use with the built-in dev tools. The web version builds it for web, with logging, sanitizing and assertions enabled. Don't run `make web-dev` directly, instead, use the following:

```console
bin/web-dev
```

This will run `make web-dev` and serve it with `python -m http.server` on the port set on your `.env` file so you can run it on your browser.

To format and lint the whole project, run the following:

```console
bin/format
bin/lint
```

## Working with a web build

To make a web build, you will need to have [emsdk](https://emscripten.org/docs/getting_started/downloads.html) version 4.0.10 and `python`. Follow the link on how to install it in this version. Then, update you `.env` file with the directory you installed `emsdk` in.

If you can't compile either `raylib` or the game, pay attention to the error messages, might be some of these binaries that can't be found. Something might not be on your `PATH` or you did not set your `.env` correctly.

## License

This is an unlicensed project, meaning you can do whatever you want with it.

For more information, please refer to <https://unlicense.org>
