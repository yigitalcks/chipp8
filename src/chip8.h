#ifndef CHIP8_H
#define CHIP8_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <random>

class Chip8 {
public:
	static constexpr int MEMORY_SIZE{4096};
	static constexpr int N_REGISTERS{16};
	static constexpr int N_KEY{16};

	static constexpr uint16_t STACK_START_ADDRESS{0x0200};
	static constexpr uint16_t FONT_DATA_ADDRESS{0x0190};
	static constexpr uint16_t PROGRAM_ADDRESS{0x0200};
	static constexpr uint16_t PROGRAM_END_ADDRESS{0x0E8F};
	static constexpr uint16_t PROGRAM_SIZE_LIMIT{PROGRAM_END_ADDRESS - PROGRAM_ADDRESS};

	static constexpr int DISPLAY_WIDTH{64};
	static constexpr int DISPLAY_HEIGHT{32};
	static constexpr int FONT_SPRITE_SIZE{5};

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
	static constexpr std::array<std::pair<SDL_Scancode, uint8_t>, N_KEY> kKeyMap{{
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

private:
	static constexpr std::array<uint8_t, 80> kFontData{{
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
	}};

	uint16_t m_i{};
	uint16_t m_sp{STACK_START_ADDRESS};

	uint8_t m_delayTimer{};
	uint8_t m_soundTimer{};

	// Holds whether a key (0-F) is pressed
	std::bitset<N_KEY> m_keys;

	// General-purpose registers
	std::array<uint8_t, N_REGISTERS> V{};

	std::mt19937 m_gen;
	std::uniform_int_distribution<int> m_distrib;

	std::array<uint8_t, MEMORY_SIZE> m_memory{};
	std::array<uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT> m_display{};
	uint16_t m_pc{PROGRAM_ADDRESS};

	void I_00E0();                      			// CLS
	void I_00EE();                      			// RET
	void I_0NNN(uint16_t nnn);          			// DW #0NNN
	void I_1NNN(uint16_t nnn);          			// JP NNN
	void I_2NNN(uint16_t nnn);          			// CALL NNN
	void I_3XNN(uint8_t x, uint8_t nn); 			// SE VX, NN
	void I_4XNN(uint8_t x, uint8_t nn); 			// SNE VX, NN
	void I_5XY0(uint8_t x, uint8_t y);  			// SE VX, VY
	void I_6XNN(uint8_t x, uint8_t nn); 			// LD VX, NN
	void I_7XNN(uint8_t x, uint8_t nn); 			// ADD VX, NN
	void I_8XY0(uint8_t x, uint8_t y);  			// LD VX, VY
	void I_8XY1(uint8_t x, uint8_t y);  			// OR VX, VY
	void I_8XY2(uint8_t x, uint8_t y);  			// AND VX, VY
	void I_8XY3(uint8_t x, uint8_t y);  			// XOR VX, VY
	void I_8XY4(uint8_t x, uint8_t y);  			// ADD VX, VY
	void I_8XY5(uint8_t x, uint8_t y);  			// SUB VX, VY
	void I_8XY6(uint8_t x, uint8_t y);  			// SHR VX {, VY}
	void I_8XY7(uint8_t x, uint8_t y);  			// SUBN VX, VY
	void I_8XYE(uint8_t x, uint8_t y);  			// SHL VX {, VY}
	void I_9XY0(uint8_t x, uint8_t y);  			// SNE VX, VY

	void I_ANNN(uint16_t nnn); 						// LD I, NNN
	void I_BNNN(uint16_t nnn); 						// JP V0, NNN

	void I_CXNN(uint8_t x, uint8_t nn);           	// RND VX, NN
	void I_DXYN(uint8_t x, uint8_t y, uint8_t n); 	// DRW VX, VY, N

	void I_EX9E(uint8_t x); 						// SKP VX
	void I_EXA1(uint8_t x); 						// SKNP VX
	void I_FX07(uint8_t x); 						// LD VX, DT
	bool I_FX0A(uint8_t x); 						// LD VX, K
	void I_FX15(uint8_t x); 						// LD DT, VX
	void I_FX18(uint8_t x); 						// LD ST, VX
	void I_FX1E(uint8_t x); 						// ADD i,VX
	void I_FX29(uint8_t x); 						// LD F, VX
	void I_FX33(uint8_t x); 						// LD B, VX
	void I_FX55(uint8_t x); 						// LD [I], VX
	void I_FX65(uint8_t x); 						// LD VX, [I]

public:
	Chip8();

	bool loadROM(const std::filesystem::path &path);
	uint16_t fetch();
	void decode_and_execute(uint16_t instruction);
	void step();

	const uint8_t* getDisplayBuffer() const { return m_display.data(); }

	uint8_t getDelayTimer() { return m_delayTimer; }
	uint8_t getSoundTimer() { return m_soundTimer; }

	void setKey(uint8_t keyIndex, bool pressed) {
		if (keyIndex < N_KEY) {
			m_keys[keyIndex] = pressed;
		}
	}

	void decreaseDelayTimer() {
		if (m_delayTimer > 0)
			m_delayTimer--;
	}
	void decreaseSoundTimer() {
		if (m_soundTimer > 0)
			m_soundTimer--;
	}
};

#endif
