#include "constants.h"
#include "emulator.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path to rom>\n";
    return EXIT_FAILURE;
  }

  Emulator emulator;
  if (!emulator.run(argv[1])) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}