#include "chip8.h"

Chip8::Chip8() : m_gen(std::random_device{}()), m_distrib(0, 255) {
  std::memcpy(memory + FONT_DATA_ADDRESS, kFontData, sizeof(kFontData));
}

void Chip8::I_00E0() { display.fill(0); }

void Chip8::I_00EE() {
  uint16_t ret = (memory[m_sp] << 8) | memory[m_sp + 1];
  m_sp += 2;

  pc = ret;
}

// Deprecated
void Chip8::I_0NNN(uint16_t nnn) {
  // Not Implemented
}

void Chip8::I_1NNN(uint16_t nnn) { pc = nnn; }

void Chip8::I_2NNN(uint16_t nnn) {
  m_sp -= 2;
  memory[m_sp] = (pc >> 8) & 0x00FF;
  memory[m_sp + 1] = pc & 0x00FF;

  pc = nnn;
}

void Chip8::I_3XNN(uint8_t x, uint8_t nn) {
  if (V[x] == nn)
    pc += 2;
}

void Chip8::I_4XNN(uint8_t x, uint8_t nn) {
  if (V[x] != nn)
    pc += 2;
}

void Chip8::I_5XY0(uint8_t x, uint8_t y) {
  if (V[x] == V[y])
    pc += 2;
}

void Chip8::I_6XNN(uint8_t x, uint8_t nn) { V[x] = nn; }

void Chip8::I_7XNN(uint8_t x, uint8_t nn) { V[x] += nn; }

void Chip8::I_8XY0(uint8_t x, uint8_t y) { V[x] = V[y]; }

void Chip8::I_8XY1(uint8_t x, uint8_t y) { V[x] |= V[y]; }

void Chip8::I_8XY2(uint8_t x, uint8_t y) { V[x] &= V[y]; }

void Chip8::I_8XY3(uint8_t x, uint8_t y) { V[x] ^= V[y]; }

void Chip8::I_8XY4(uint8_t x, uint8_t y) {
  // Set VF to 1 if a carry occurs
  V[0xF] = (V[x] > std::numeric_limits<uint8_t>::max() - V[y]);

  V[x] += V[y];
}

void Chip8::I_8XY5(uint8_t x, uint8_t y) {
  if (V[x] >= V[y])
    V[0xF] = 1;
  else {
    V[0xF] = 0;
  }

  V[x] -= V[y];
}

void Chip8::I_8XY6(uint8_t x, uint8_t y) {
  V[x] = V[y];

  uint8_t shifted_out = V[x] & 0x01;
  V[x] >>= 1;
  V[0xF] = shifted_out;
}

void Chip8::I_8XY7(uint8_t x, uint8_t y) {
  if (V[y] >= V[x])
    V[0xF] = 1;
  else {
    V[0xF] = 0;
  }

  V[x] = V[y] - V[x];
}

void Chip8::I_8XYE(uint8_t x, uint8_t y) {
  V[x] = V[y];

  uint8_t shifted_out = V[x] & 0x80;
  V[x] <<= 1;
  V[0xF] = shifted_out;
}

void Chip8::I_9XY0(uint8_t x, uint8_t y) {
  if (V[x] != V[y])
    pc += 2;
}

void Chip8::I_ANNN(uint16_t nnn) { m_i = nnn; }

void Chip8::I_BNNN(uint16_t nnn) { pc = nnn + V[0]; }

void Chip8::I_CXNN(uint8_t x, uint8_t nn) { V[x] = m_distrib(m_gen) & nn; }

void Chip8::I_DXYN(uint8_t x, uint8_t y, uint8_t n) {
  uint8_t edgeH = std::min(int(V[x] % DISPLAY_WIDTH + 8), DISPLAY_WIDTH);
  uint8_t edgeV = std::min(int(V[y] % DISPLAY_HEIGHT + n), DISPLAY_HEIGHT);

  V[0xF] = 0;

  uint8_t c{0};
  for (uint8_t i = V[y] % DISPLAY_HEIGHT; i < edgeV; i++) {
    uint8_t spriteRow{memory[m_i + c]};

    uint8_t shiftC = 7;
    for (uint8_t j = V[x] % DISPLAY_WIDTH; j < edgeH; j++) {
      uint8_t spritePixel = (spriteRow >> shiftC) & 0x01;
      uint8_t displayPixel = display[i * DISPLAY_WIDTH + j];

      V[0xF] |= (spritePixel && displayPixel);

      display[i * DISPLAY_WIDTH + j] = spritePixel ^ displayPixel;
      shiftC--;
    }
    c++;
  }
}

void Chip8::I_EX9E(uint8_t x) {
  if (m_keys[V[x] & 0x0F]) {
    pc += 2;
  }
}

void Chip8::I_EXA1(uint8_t x) {
  if (!m_keys[V[x] & 0x0F]) {
    pc += 2;
  }
}

void Chip8::I_FX07(uint8_t x) { V[x] = m_delayTimer; }

bool Chip8::I_FX0A(uint8_t x) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
      for (const auto &[sdl_key, chip8_key] : kKeyMap) {
        if (sdl_key == e.key.scancode) {
          V[x] = chip8_key;
          return true;
        }
      }
    }
  }
  return false;
}

void Chip8::I_FX15(uint8_t x) { m_delayTimer = V[x]; }

void Chip8::I_FX18(uint8_t x) { m_soundTimer = V[x]; }

void Chip8::I_FX1E(uint8_t x) { m_i += V[x]; }

void Chip8::I_FX29(uint8_t x) {
  m_i = FONT_DATA_ADDRESS + (V[x] & 0x0F) * FONT_SPRITE_SIZE;
}

void Chip8::I_FX33(uint8_t x) {
  memory[m_i] = V[x] / 100;
  memory[m_i + 1] = (V[x] / 10) % 10;
  memory[m_i + 2] = V[x] % 10;
}

void Chip8::I_FX55(uint8_t x) {
  for (uint8_t i = 0; i <= x; i++) {
    memory[m_i + i] = V[i];
  }

  m_i += x + 1;
}

void Chip8::I_FX65(uint8_t x) {
  for (uint8_t i = 0; i <= x; i++) {
    V[i] = memory[m_i + i];
  }

  m_i += x + 1;
}