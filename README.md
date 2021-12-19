# Storm DB

Very basic implementation of an SQL database in C++. Done as learning excersise, not for production use.

## Features:
- Basic SQL parsing (lexing and parsing)
- SQL compilation (validate types and expressions against database schema)
- Indexes

## Installing:
1. Download or clone this repository (use flag `--recurse-submodules` or `--recursive` to include submodules).

## Building on Windows:
1. Run `Scripts/Win-GenProjects.bat` and build the solution using Visual Studio 2019.
2. Build outputs are located in the `bin` directory.

## Building on Linux:
1. Run `Scripts/Linux-GenProjects.sh` to generate the Makefiles.
2. Run `make -j<number_of_cores> StormDB-Cli config=dist` to build Storm.
3. Build outputs are located in the `bin` directory.
