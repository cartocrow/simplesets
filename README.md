# CartoCrow: SimpleSets
[![arXiv](https://img.shields.io/badge/arXiv-2407.14433-b31b1b.svg)](https://arxiv.org/abs/2407.14433)

This repository contains an implementation of SimpleSets.
Given a set of points in the plane that each belong to one category, SimpleSets creates simple shapes that enclose patterns in the data.
The visualization is described in more detail in the corresponding paper:
> Steven van den Broek, Wouter Meulemans, Bettina Speckmann. _SimpleSets: Capturing Categorical Point Patterns with Simple Shapes_. 
> IEEE Transactions on Visualization and Computer Graphics, 2025. [10.1109/TVCG.2024.3456168](https://doi.org/10.1109/TVCG.2024.3456168)

## Dependencies
This module depends on the following libraries:

* [Cartocrow/core](https://github.com/cartocrow/core)

CartoCrow/core can be cloned and built using CMake as described in <https://github.com/cartocrow/core>.

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
