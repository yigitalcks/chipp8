#include <cstdlib>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "LTexture.h"

constexpr int kScreenWidth  { 1280 };
constexpr int kScreenHeight { 720 };

static SDL_Window *window         {nullptr};
static SDL_Renderer *renderer     {nullptr};

int main(int argc, char *argv[])
{
    SDL_SetAppMetadata("Primitives", "1.0", "zort");
    
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    if(!SDL_CreateWindowAndRenderer("Primitives", kScreenWidth, kScreenHeight, SDL_WINDOW_RESIZABLE, &window, &renderer)){
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetRenderLogicalPresentation(renderer, kScreenWidth, kScreenHeight, SDL_LOGICAL_PRESENTATION_DISABLED);


    if(SDL_GetBasePath() == nullptr) {
        SDL_Log("SDL_GetBasePath: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    
    std::string resourcePath {SDL_GetBasePath()};
    std::string imagePath {resourcePath + "lena.bmp"};

    LTexture texture{renderer};
    if (!texture.load(imagePath))
    {
        return EXIT_FAILURE;
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
            case SDL_EVENT_KEY_DOWN:
                if(e.key.key == SDLK_ESCAPE)
                    isRunning = false;
                break;
            default:
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 33, 33, 33, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        texture.render(0.f, 0.f);
        
        SDL_RenderPresent(renderer);
    }


    texture.clean();
    SDL_DestroyWindow(window);
    window = nullptr;
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;

    SDL_Quit();

    return EXIT_SUCCESS;
}