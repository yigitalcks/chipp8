#include "chip8.h"

Chip8::Chip8()
    : m_gen(std::random_device{}()), 
      m_distrib(0, 255) 
{
    std::memcpy(memory + FONT_DATA_ADDRESS, kFontData, sizeof(kFontData));
}

void Chip8::I_00E0()
{
    std::memset(memory, 0, sizeof(memory));
}

void Chip8::I_00EE()
{
    uint16_t ret = (memory[m_sp] << 8) | memory[m_sp + 1];
    m_sp += 2;

    pc = ret;
}

// Deprecated
void Chip8::I_0NNN()
{
}

void Chip8::I_1NNN(uint16_t instruction)
{
    pc = instruction & 0x0FFF;
}

void Chip8::I_2NNN(uint16_t instruction)
{
    m_sp -= 2;
    memory[m_sp] = (pc >> 8) & 0x00FF;
    memory[m_sp + 1] = pc & 0x00FF;

    pc = instruction & 0x0FFF;
}

void Chip8::I_3XNN(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t nn = instruction & 0x00FF;

    if(V[x] == nn)
        pc += 2;
    
}

void Chip8::I_4XNN(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t nn = instruction & 0x00FF;

    if(V[x] != nn)
        pc += 2;
    
}

void Chip8::I_5XY0(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F ;
    uint8_t y = (instruction >> 4) & 0x000F ;

    if(V[x] == V[y])
        pc += 2;
}

void Chip8::I_6XNN(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t nn = instruction & 0x00FF;

    V[x] = nn;
}

void Chip8::I_7XNN(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t nn = instruction & 0x00FF;

    V[x] += nn;
}

void Chip8::I_8XY0(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    V[x] = V[y];
}

void Chip8::I_8XY1(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    V[x] |= V[y];
}

void Chip8::I_8XY2(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    V[x] &= V[y];
}

void Chip8::I_8XY3(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    V[x] ^= V[y];
}

void Chip8::I_8XY4(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    // Set VF to 1 if a carry occurs
    V[0xF] = (V[x] > std::numeric_limits<uint8_t>::max() - V[y]);

    V[x] += V[y];
}

void Chip8::I_8XY5(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    if(V[x] >= V[y])
        V[0xF] = 1;
    else {
        V[0xF] = 0;
    }
    
    V[x] -= V[y];
}

void Chip8::I_8XY6(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    V[x] = V[y];

    V[0xF] = V[x] & 0x01;
    V[x] >>= 1;
}

void Chip8::I_8XY7(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    if(V[y] >= V[x])
        V[0xF] = 1;
    else {
        V[0xF] = 0;
    }

    V[x] = V[y] - V[x];
}

void Chip8::I_8XYE(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    V[x] = V[y];

    V[0xF] = V[x] & 0x80;
    V[x] <<= 1;
}

void Chip8::I_9XY0(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;

    if(V[x] != V[y])
        pc += 2;
}

void Chip8::I_ANNN(uint16_t instruction)
{
    m_i = instruction & 0x0FFF;
}

void Chip8::I_BNNN(uint16_t instruction)
{
    pc = instruction & 0x0FFF + V[0];
}


void Chip8::I_CXNN(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t nn = instruction & 0x00FF;

    V[x] = m_distrib(m_gen) & nn;
}

void Chip8::I_DXYN(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    uint8_t y = (instruction >> 4) & 0x000F;
    uint8_t n = instruction & 0x000F;

    uint8_t edgeH = std::max(int(V[x] % DISPLAY_WIDTH + sizeof(uint8_t)), DISPLAY_WIDTH);
    uint8_t edgeV = std::max(int(V[y] % DISPLAY_HEIGHT + n), DISPLAY_HEIGHT);

    V[0xF] = 0;

    uint8_t c { 0 };
    for (uint8_t i = V[y]; i < edgeV; i++)
    {
        uint8_t spriteRow { memory[m_i + c] };

        uint8_t shiftC { sizeof(uint8_t) - 1 };
        for (uint8_t j = V[x]; j < edgeH; j++)
        {
            uint8_t spritePixel = (spriteRow >> shiftC) & 0x01;
            uint8_t displayPixel = m_display[i][j];

            V[0xF] = spritePixel && displayPixel;

            m_display[i][j] = spritePixel ^ displayPixel; 
            shiftC--;
        }
        c++;
    }
}

void Chip8::I_EX9E(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;

    if (m_keys[V[x] & 0x0F])
    {
        pc += 2;
    }
}

void Chip8::I_EXA1(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;

    if (!m_keys[V[x] & 0x0F])
    {
        pc += 2;
    }
}

void Chip8::I_FX07(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    V[x] = m_delayTimer;
}

bool Chip8::I_FX0A(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_KEY_DOWN)
        {
            for (const auto& [sdl_key, chip8_key] : kKeyMap)
            {
                if(sdl_key == e.key.scancode) {
                    V[x] = chip8_key;
                    return true;
                }
            }
        }
    }
    // TODO dönüş değerini handle et!
    return false;
}

void Chip8::I_FX15(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;

    V[x] = m_delayTimer;
}

void Chip8::I_FX18(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;

    V[x] = m_soundTimer;
}

void Chip8::I_FX1E(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;

    m_i += V[x];
}

void Chip8::I_FX29(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;

    m_i = memory[FONT_DATA_ADDRESS + (V[x] & 0x0F) * FONT_SPRITE_SIZE];
}

void Chip8::I_FX33(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;
    
    memory[m_i] = V[x] / 100;
    memory[m_i + 1] = (V[x] / 10) % 10;
    memory[m_i + 1] = V[x] % 10;
}

void Chip8::I_FX55(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;

    for (uint8_t i = 0; i <= x; i++)
    {
        memory[m_i + i] = V[i];
    }

    m_i += x + 1;
}

void Chip8::I_FX65(uint16_t instruction)
{
    uint8_t x = (instruction >> 8) & 0x000F;

    for (uint8_t i = 0; i <= x; i++)
    {
        V[i] = memory[m_i + i];
    }
    
    m_i += x + 1;
}