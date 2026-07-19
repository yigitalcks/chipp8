#ifndef EMULATOR_H
#define EMULATOR_H

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <SDL3/SDL.h>
#include "constants.h"
#include "chip8.h"

#define FREQUENCY 1000 // 1Mhz

class Emulator
{
private:
    SDL_Window* m_window { nullptr };
    SDL_Renderer* m_renderer { nullptr };
    SDL_Event m_event;
    Chip8 m_chip8{};

    bool load(const std::filesystem::path& path);
    uint16_t fetch();
    void decode(uint16_t instruction);
    void execute();

public:
    Emulator();
    ~Emulator();

    bool run(const std::filesystem::path& path);
};

#endif // EMULATOR_H
