#include "emulator.h"

Emulator::Emulator()
{
}

Emulator::~Emulator()
{
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Emulator::initialize()
{
    if (!SDL_CreateWindowAndRenderer("Hello World", 800, 600, SDL_WINDOW_BORDERLESS, &m_window, &m_renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool Emulator::isRunning() const
{
    return m_isRunning;
}
