# CartoCrow: SimpleSets
[![arXiv](https://img.shields.io/badge/arXiv-2407.14433-b31b1b.svg)](https://arxiv.org/abs/2407.14433)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.20816553.svg)](https://doi.org/10.5281/zenodo.20816553)

This repository contains an implementation of SimpleSets.
Given a set of points in the plane that each belong to one category, SimpleSets creates simple shapes that enclose patterns in the data.
The visualization is described in more detail in the corresponding paper:
> Steven van den Broek, Wouter Meulemans, Bettina Speckmann. _SimpleSets: Capturing Categorical Point Patterns with Simple Shapes_. 
> IEEE Transactions on Visualization and Computer Graphics, 2025. [10.1109/TVCG.2024.3456168](https://doi.org/10.1109/TVCG.2024.3456168)

## Dependencies
This module depends on the following libraries:

* [Cartocrow/core](https://github.com/cartocrow/core) (v0.1.0)

CartoCrow/core can be cloned and built using CMake as described in <https://github.com/cartocrow/core>.

## Compiling

### Emscripten
To compile SimpleSets to WebAsssembly with Emscripten, do the following.
1. Compile [cartocrow/core](https://github.com/cartocrow/core) with Emscripten.
2. Run the following commands; the placeholder paths refer to those in the [cartocrow/core](https://github.com/cartocrow/core) Emscripten installation instructions.
```sh
emcmake cmake -S . -B wasm_build -DCMAKE_BUILD_TYPE=Release -DCartoCrow_DIR=<path/to/compiled_webassembly/lib/cmake/CartoCrow> -DEMSCRIPTEN_INCLUDE_DIR=<path/to/cartocrow_wasm_files>
cmake --build wasm_build
```
This creates a `wasm_build/wasm_frontend/simplesets_wasm.js` file that exposes a JavaScript API.

## Usage
The program receives as input points in the plane that each have exactly one category.
A data format we use a .txt file that contains a point on each line in the format `c x y` where `c` is a non-negative 
integer representing the category of the point, and `x` and `y` are the coordinates of the point.

The data folder contains example data.
There is both a graphical and a command-line interface (GUI and CLI).
To load data in the GUI, press the corresponding button.
For the CLI, pass a JSON file that specifies the input file and all options (see the examples in the `data` folder).

## License

Copyright (c) 2026 TU Eindhoven
Licensed under the GPLv3.0 license. See LICENSE for details.
