# grey

Minimalistic cross-platform UI framework based on [ImGui](https://github.com/ocornut/imgui) at first but is slowly diverging into something else.

It's basically ImGui shaped into making desktop applications, because ImGui can be used for anything, and this library simply makes it simple to start off with desktop.

## Features

- Supports High DPI out of the box.
- Replaces default font with Roboto, and adds support for an icon font out of the box.
- Completely stateless i.e. no widgets, state synchronisation and so on.

## Technical remarks

### Font choices

Grey uses [Google Material Icons](https://fonts.google.com/icons?icon.set=Material+Icons). Initially, I was using FontAwesome, and then ForkAwesome. The last wasn't updated for quite some time, and I had issues with the first.

[This repo](https://github.com/juliettef/IconFontCppHeaders) contains convenient headers, which can be copied into *fonts* subfolder. To generate the inline, use:

```
.\binary_to_compressed.exe .\fa-regular-400.ttf font_awesome_regular_400 > font_awesome_6_regular_400.inl
```

The rest of the code in `font_loader.hpp` shows how to use it.