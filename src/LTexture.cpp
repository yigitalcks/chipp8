#include "LTexture.h"

LTexture::LTexture(SDL_Renderer* renderer)
{
    m_renderer = renderer;
}

LTexture::~LTexture()
{
    clean();
}

bool LTexture::load(std::string path)
{
    // clean the texture if exists
    clean();

    SDL_Surface* surface = IMG_Load(path.c_str());
    if(surface == nullptr) {
        SDL_Log("LTexture, failed to load image:%s", SDL_GetError());
        return false;
    }

    m_texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (m_texture == nullptr) {
        SDL_Log("LTexture, failed to create texture from surface:%s", SDL_GetError());
        return false;
    }

    m_width = surface->w;
    m_height = surface->h;

    SDL_DestroySurface(surface);

    return true;
}

void LTexture::clean()
{
    SDL_DestroyTexture( m_texture );
    m_texture = nullptr;
    m_width = 0;
    m_height = 0;
}

void LTexture::render(float x, float y)
{
    SDL_FRect dstRect{x, y, static_cast<float>(m_width), static_cast<float>(m_height)};

    SDL_RenderTexture(m_renderer, m_texture, nullptr, &dstRect);
}

int LTexture::getWidth()
{
    return m_width;
}

int LTexture::getHeight()
{
    return m_height;
}

bool LTexture::isLoaded()
{
    return m_texture != nullptr;
}
