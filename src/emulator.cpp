#include "emulator.h"

Emulator::Emulator() {
    SDL_SetAppMetadata("Chipp8", "0.5", "zort");
    
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    }

    if(!SDL_CreateWindowAndRenderer("Chipp8", constants::windowWidth, constants::windowHeight, SDL_WINDOW_RESIZABLE, &m_window, &m_renderer)){
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    }

    SDL_SetRenderLogicalPresentation(m_renderer, constants::windowWidth, constants::windowHeight, SDL_LOGICAL_PRESENTATION_DISABLED);
}

Emulator::~Emulator() {  
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
    SDL_DestroyRenderer(m_renderer);
    m_renderer = nullptr;
    SDL_Quit();
}

bool Emulator::load(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Loading Rom failed";
        return false;
    }

    size_t size = std::filesystem::file_size(path);
    if (size > PROGRAM_SIZE_LIMIT) {
        std::cerr << "Rom size limit has been exceeded! Rom size limit: " << PROGRAM_SIZE_LIMIT << " bytes";
        return false;
    }

    // reading rom into memory address 0x200
    file.read(reinterpret_cast<char*>(&m_chip8.memory[PROGRAM_ADDRESS]), static_cast<std::streamsize>(size));

    if (!file && !file.eof())
        std::cerr << "Reading error: " << path.string();
        return false;

    return true;
}

uint16_t Emulator::fetch()
{
    uint16_t instruction = (m_chip8.memory[m_chip8.pc] << 8) | m_chip8.memory[m_chip8.pc + 1];
    return instruction;
}

void Emulator::decode(uint16_t instruction)
{
    
}

void Emulator::execute()
{
}

bool Emulator::run(const std::filesystem::path& path) {

    if (!load(path))
    {
        return false;
    }

    uint16_t instruction { fetch() };

    

}