#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>

class LTexture {
public:
    LTexture(SDL_Renderer* renderer);
    ~LTexture();

    bool load(std::string path);
    void clean();

    void render(float x, float y);

    int getWidth();
    int getHeight();
    bool isLoaded();

private:
    SDL_Renderer* m_renderer{nullptr};
    SDL_Texture* m_texture{nullptr};

    int m_width{0};
    int m_height{0};
};

#endif