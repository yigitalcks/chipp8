#ifndef EMULATOR_H
#define EMULATOR_H

#include <SDL3/SDL.h>

class Emulator {
    private:

    SDL_Window* m_window {nullptr};
    SDL_Renderer* m_renderer {nullptr};
    bool m_isRunning {true};

    public:

    Emulator();
    ~Emulator();

    bool initialize();
    bool isRunning() const;

};

#endif
