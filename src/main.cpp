#include <iostream>
#include <cstdlib>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "constants.h"

static SDL_Window *window         {nullptr};
static SDL_Renderer *renderer     {nullptr};

int main(int argc, char *argv[])
{
    
    if (argc != 2)
    {
        std::cerr << "Usage: %s <path to rom>";
    }
    
    

    bool isRunning {true};
    while(isRunning) {

        SDL_Event e{};
        while(SDL_PollEvent(&e)) {
            switch (e.type)
            {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;
        }

        SDL_SetRenderDrawColor(renderer, 33, 33, 33, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        
        SDL_RenderPresent(renderer);
        }
    }

    return EXIT_SUCCESS;
}