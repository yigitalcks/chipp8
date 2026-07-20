#ifndef EMULATOR_H
#define EMULATOR_H

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <SDL3/SDL.h>
#include "constants.h"
#include "chip8.h"

#define CPU_HZ          1000        // 1Mhz
#define TIMER_HZ        60
#define SCREEN_HZ       60          // FPS
#define NS_PER_SECOND   1000000000

class Emulator
{
private:
    SDL_Window* m_window { nullptr };
    SDL_Renderer* m_renderer { nullptr };
    SDL_AudioStream* m_audioStream { nullptr };
    SDL_Event m_event;
    Chip8 m_chip8{};

    void initAudio();

    static constexpr uint64_t nsPerCycle = NS_PER_SECOND / CPU_HZ;
    static constexpr uint64_t nsPerTimerTick = NS_PER_SECOND / TIMER_HZ;
    static constexpr uint64_t nsPerFrame = NS_PER_SECOND / SCREEN_HZ;

    bool load(const std::filesystem::path& path);
    uint16_t fetch();
    void decode_and_execute(uint16_t instruction);

public:
    Emulator();
    ~Emulator();

    bool run(const std::filesystem::path& path);
};

#endif // EMULATOR_H
