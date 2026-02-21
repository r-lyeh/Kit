# Hurdyl

A pure Lua port of the Lua library for [Hurdy](https://hurdy.apicici.com).

## Installation and use

The library can be installed by downloading a [release](https://codeberg.org/apicici/hurdyl/releases) and copying the `hurdyl` folder containing the Lua files directly inside a Lua project.

Alternatively, it can be installed through LuaRocks with

```
luarocks install hurdy
```

Refer to the documentation at https://hurdy.apicici.com for information about the language and how to use the library.

## Source

The library is written in Hurdy itself. The original Hurdy files are in the `src` folder. The Lua files in the `hurdyl` folder have been compiled using the [`hurdyc` compiler](https://codeberg.org/apicici/hurdy).

## Third-party code

The project contains code from the following projects:

- Crafting Interpreters
	+ Website: https://github.com/munificent/craftinginterpreters
	+ License: MIT License
	+ Copyright (c) 2015 Robert Nystrom
	+ Note: the scanner and parser in `hurdy` are inspired by and follow the same code structure as `jlox`, translated to C++