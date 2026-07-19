#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <cstring>
#include <limits>
#include <random>
#include <bitset>
#include <array>
#include <algorithm>
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>

// 0x190 - 0x1DF FontData
// 0x1E0 - 0x1FF Stack
// 0x200 - 0xE8F Programs

#define MEMORY_SIZE 4096
#define N_REGISTERS 16
#define N_KEY       16

#define STACK_START_ADDRESS (0x0200)
#define FONT_DATA_ADDRESS   (0x0190)
#define PROGRAM_ADDRESS     (0x0200)
#define PROGRAM_END_ADDRESS (0x0E8F)
#define PROGRAM_SIZE_LIMIT   (PROGRAM_ADDRESS - PROGRAM_END_ADDRESS)

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define FONT_SPRITE_SIZE    5

class Chip8
{
private:

    static constexpr uint8_t kFontData[] {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    /*
     *  +---+---+---+---+     +---+---+---+---+
     *  | 1 | 2 | 3 | C |     | 1 | 2 | 3 | 4 |
     *  +---+---+---+---+     +---+---+---+---+
     *  | 4 | 5 | 6 | D |     | Q | W | E | R |
     *  +---+---+---+---+ ==> +---+---+---+---+
     *  | 7 | 8 | 9 | E |     | A | S | D | F |
     *  +---+---+---+---+     +---+---+---+---+
     *  | A | 0 | B | F |     | Z | X | C | V |
     *  +---+---+---+---+     +---+---+---+---+
     */
    static constexpr std::array<std::pair<SDL_Scancode, uint8_t>, N_KEY> kKeyMap {{
        {SDL_SCANCODE_1, 0x01},
        {SDL_SCANCODE_2, 0x02},
        {SDL_SCANCODE_3, 0x03},
        {SDL_SCANCODE_Q, 0x04},
        {SDL_SCANCODE_W, 0x05},
        {SDL_SCANCODE_E, 0x06},
        {SDL_SCANCODE_A, 0x07},
        {SDL_SCANCODE_S, 0x08},
        {SDL_SCANCODE_D, 0x09},
        {SDL_SCANCODE_Z, 0x0A},
        {SDL_SCANCODE_X, 0x00},
        {SDL_SCANCODE_C, 0x0B},
        {SDL_SCANCODE_4, 0x0C},
        {SDL_SCANCODE_R, 0x0D},
        {SDL_SCANCODE_F, 0x0E},
        {SDL_SCANCODE_V, 0x0F},
    }};

    std::array<std::bitset<DISPLAY_WIDTH>, DISPLAY_HEIGHT> m_display {};

    uint16_t m_i {};
    uint16_t m_sp { STACK_START_ADDRESS };

    uint8_t m_delayTimer {};
    uint8_t m_soundTimer {};

    // Holds wheter is a key(0-F) pressed
    std::bitset<N_KEY> m_keys; 

    // general-purpose registers
    uint8_t V[N_REGISTERS];

    std::mt19937 m_gen;
    std::uniform_int_distribution<int> m_distrib;
public:

    uint8_t memory[MEMORY_SIZE] {};
    uint16_t pc {PROGRAM_ADDRESS};

    Chip8();
    
    // 0 Series
    void I_00E0();                      // CLS
    void I_00EE();                      // RET
    void I_0NNN();                      // DW #0NNN
    void I_1NNN(uint16_t instruction);  // JP NNN
    void I_2NNN(uint16_t instruction);  // CALL NNN
    void I_3XNN(uint16_t instruction);  // SE VX, NN
    void I_4XNN(uint16_t instruction);  // SNE VX, NN
    void I_5XY0(uint16_t instruction);  // SE VX, VY
    void I_6XNN(uint16_t instruction);  // LD VX, NN
    void I_7XNN(uint16_t instruction);  // ADD VX, NN
    void I_8XY0(uint16_t instruction);  // LD VX, VY
    void I_8XY1(uint16_t instruction);  // OR VX, VY
    void I_8XY2(uint16_t instruction);  // AND VX, VY
    void I_8XY3(uint16_t instruction);  // XOR VX, VY
    void I_8XY4(uint16_t instruction);  // ADD VX, VY
    void I_8XY5(uint16_t instruction);  // SUB VX, VY
    void I_8XY6(uint16_t instruction);  // SHR VX {, VY}
    void I_8XY7(uint16_t instruction);  // SUBN VX, VY
    void I_8XYE(uint16_t instruction);  // SHL VX {, VY}
    void I_9XY0(uint16_t instruction);  // SNE VX, VY

    void I_ANNN(uint16_t instruction);  // LD I, NNN
    void I_BNNN(uint16_t instruction);  // JP V0, NNN

    void I_CXNN(uint16_t instruction);  // RND VX, NN
    void I_DXYN(uint16_t instruction);  // DRW VX, VY, N

    void I_EX9E(uint16_t instruction);  // SKP VX
    void I_EXA1(uint16_t instruction);  // SKNP VX

    void I_FX07(uint16_t instruction);  // LD VX, DT
    bool I_FX0A(uint16_t instruction);  // LD VX, K
    void I_FX15(uint16_t instruction);  // LD DT, VX
    void I_FX18(uint16_t instruction);  // LD ST, VX
    void I_FX1E(uint16_t instruction);  // ADD i,VX
    void I_FX29(uint16_t instruction);  // LD F, VX
    void I_FX33(uint16_t instruction);  // LD B, VX
    void I_FX55(uint16_t instruction);  // LD [I], VX
    void I_FX65(uint16_t instruction);  // LD VX, [I]
};

#endif