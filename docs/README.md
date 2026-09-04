# Grey

![logo](icon64.png)

> [!WARNING]
> This project is not yet completely ready for public consumption as a product. It's mostly used internally by 10+ projects and some public projects that are either open or closed source.

Grey is a minimalistic, blazing fast, disposable, self-container, cross-platform, zero-configuration UI library for C++ (C# wrapper available) that frees up developers' grey matter to do something that matters.

Grey is designed to display the user interface in one line of code with no ceremony at all, mostly for debugging or diagnostics, or small internal tools. It is not a full-featured UI framework for end-user applications. However, you can [try](https://github.com/aloneguid/bt) to do that as well.

It's based on the awesome [ImGui](https://github.com/ocornut/imgui) framework and follows its design principles, specifically the immediate mode paradigm.

## Features

- Cross-platform:
  - Windows x64
  - Linux x64
  - Apple Silicon
  - Intel Mac
- Completely stateless i.e., no widgets, state synchronization and so on.
- Looks beautiful by default i.e., renders in high DPI, using system fonts and colors, supports themes and material icons.

## Getting started

To get started in C++, include this repository as a git submodule and follow the examples. For a real-life, production sample, check out [bt](https://github.com/aloneguid/bt).

To get started in C#, reference [the GreyMatter](https://www.nuget.org/packages/GreyMatter/) package, which includes C# interface and pre-built binaries for Windows and Linux.