# Chipp8

A simple Chip-8 interpreter targeting the original instruction set developed for the 1977 COSMAC VIP computer. Support for other Chip-8 variations or newer versions (such as SCHIP, XO-CHIP, etc.) is currently not available.

## Features

* **CPU Speed:** 1000 Hz (1000 cycles per second)
* **Timers:** 60 Hz delay and sound timers
* **Resolution:** Scaled from the original 64x32 to a 1280x720 window
* **Audio:** Mono beep tone generated using SDL3

## Requirements

* C++17 compliant compiler
* CMake (Version 3.16 or higher)
* SDL3 (Automatically compiled as a submodule; platform-specific development dependencies may be required on your system)

## Setup & Compilation

The project uses SDL3 as a git submodule. Make sure to fetch the submodules before compiling:

```bash
# Clone and update submodules
git submodule update --init --recursive

# Create build directory and compile
mkdir build && cd build
cmake ..
cmake --build .
```

## Running the Interpreter

Once the build is complete, run the interpreter by passing the path to the ROM file as an argument:

```bash
./chipp8 <path_to_rom>
```

Example usage:
```bash
./chipp8 ../breakout.ch8
```

## Keypad Mapping

The 16-key keypad of the COSMAC VIP is mapped to the QWERTY keyboard as follows:

```text
Original Keypad               QWERTY Layout
+---+---+---+---+            +---+---+---+---+
| 1 | 2 | 3 | C |            | 1 | 2 | 3 | 4 |
+---+---+---+---+            +---+---+---+---+
| 4 | 5 | 6 | D |   ===>     | Q | W | E | R |
+---+---+---+---+            +---+---+---+---+
| 7 | 8 | 9 | E |            | A | S | D | F |
+---+---+---+---+            +---+---+---+---+
| A | 0 | B | F |            | Z | X | C | V |
+---+---+---+---+            +---+---+---+---+
```

## macOS Support

On macOS, the build system configures the target as a standalone app bundle (`chipp8.app`). 

To package the application and copy all dynamic libraries (like SDL3) into the bundle for distribution, you can generate a `.dmg` installer using CPack:

```bash
cpack -G DragNDrop
```
