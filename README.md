# Langton's Ant

## Description

Simulation of [Langton's Ant](https://en.wikipedia.org/wiki/Langton%27s_ant), a cellular automaton in which
an ant "walks" over a 2D space, acting based on the tile at it's position.

The field is initialized to zeros, and incremented when the ant walks on that tile.
The values wrap around when they hit a given maximum.

The ant then acts based on the value of the tile, either turning right, left, around, or not at all.
The ant then takes a single step "forwards", and the cycle repeats.

This has shown to create interesting patterns which depend on the number of values in the field, and the decision of the ant.
The below output was created by simulation 10,000,000 iterations with the pattern given in the source.

![Output over 10,000,000 iterations](./output/output.bmp)

## Installation

### Dependencies

There are currently no dependencies of the program.
The argument parsing is done natively through `getopt`,
and the output is also native through primitive FileIO

### Makefile

The executable is built with gcc to a file named `simulate`, via the `make` command.

`make clean` will clear any object and executable files lingering, as well as resetting the output image.

## Usage

The program takes a single argument, `-i N` which represents the maximum number of iterations before termination.
If the ant reaches the outer bounds of the grid before that capacity is reached, the program will terminate early.

## TODOs

- Parameterize width and height
- Parameterize the movement pattern

