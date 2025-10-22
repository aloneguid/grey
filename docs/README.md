# Grey

![logo](icon64.png)

Grey is a minimalistic, blazing fast, disposable, cross-platform, zero configuration UI library for C++ and C# that frees up developers' grey matter to do something that matters.

Grey is designed to display user interface in one line of code with no ceremony at all, mostly for debugging or diagnostics, or small internal tools. It is not a full featured UI framework for end-user applications, however you can [try](https://github.com/aloneguid/bt) to do that as well.

It's based on the awesome [ImGui](https://github.com/ocornut/imgui) framework and follows it's design principles, specifically the immediate mode paradigm.

## Features

- Cross-platform - currently supports **Windows x64** and **Linux x64**.
- Completely stateless i.e. no widgets, state synchronisation and so on.
- Looks beautiful by default i.e. renders in high DPI, using system fonts and colours, supports themes and material icons.

## Getting started

To get started in C++, simply include this repository as a git submodule and follow the examples. For a real-life, production sample, check out [bt](https://github.com/aloneguid/bt).

To get started in C#, reference [GreyMatter](https://www.nuget.org/packages/GreyMatter/) package, which includes C# interface and pre-built binaries for Windows and Linux.

## Building

### Windows

To build native part on Windows, you only need Visual Studio 2022 or 2026 with C++ workload and CMake which you can install from Visual Studio installer.

### Linux

To build native part on Linux or WSL2, you need:
- [vcpkg](https://learn.microsoft.com/en-gb/vcpkg/get_started/get-started?pivots=shell-bash).
- CMake. `sudo apt install cmake`.
- Build tools. `sudo apt install g++ gdb make ninja-build rsync zip`.
- GLFW3 dependencies. `sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config`.

### MacOSX

I currently do not pay Apple tax, but instructions should be similar to Linux. If you want to help with MacOSX support, please open an issue. Supporting MacOSX should resort to testing Metal rendering backend and some native OS integrations around windowing.