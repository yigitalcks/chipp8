#include "chip8.h"
#include <iostream>
#include <fstream>
#include <filesystem>

Chip8::Chip8() : m_gen(std::random_device{}()), m_distrib(0, 255) {
  std::memcpy(m_memory + FONT_DATA_ADDRESS, kFontData, sizeof(kFontData));
}

bool Chip8::loadROM(const std::filesystem::path& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    std::cerr << "Loading Rom failed: " << path << "\n";
    return false;
  }

  size_t size = std::filesystem::file_size(path);
  if (size > PROGRAM_SIZE_LIMIT) {
    std::cerr << "Rom size limit has been exceeded! Rom size: " << size
              << ", limit: " << PROGRAM_SIZE_LIMIT << " bytes\n";
    return false;
  }

  file.read(reinterpret_cast<char *>(&m_memory[PROGRAM_ADDRESS]),
            static_cast<std::streamsize>(size));
  return true;
}

uint16_t Chip8::fetch() {
  if (m_pc >= MEMORY_SIZE - 1) {
    return 0;
  }
  uint16_t instruction = (m_memory[m_pc] << 8) | m_memory[m_pc + 1];
  m_pc += 2;

  return instruction;
}

void Chip8::step() {
  uint16_t instruction = fetch();
  decode_and_execute(instruction);
}

void Chip8::decode_and_execute(uint16_t instruction) {
  uint8_t op{static_cast<uint8_t>((instruction >> 12) & 0x000F)};
  uint8_t x{static_cast<uint8_t>((instruction >> 8) & 0x000F)};
  uint8_t y{static_cast<uint8_t>((instruction >> 4) & 0x000F)};
  uint8_t n{static_cast<uint8_t>((instruction) & 0x000F)};
  uint8_t nn{static_cast<uint8_t>((instruction) & 0x00FF)};
  uint16_t nnn{static_cast<uint16_t>((instruction) & 0x0FFF)};

  switch (op) {
  case 0x0:
    switch (nn) {
    case 0xE0:
      I_00E0();
      break;
    case 0xEE:
      I_00EE();
      break;
    default:
      I_0NNN(nnn);
      break;
    }
    break;

  case 0x1:
    I_1NNN(nnn);
    break;

  case 0x2:
    I_2NNN(nnn);
    break;

  case 0x3:
    I_3XNN(x, nn);
    break;

  case 0x4:
    I_4XNN(x, nn);
    break;

  case 0x5:
    if (n == 0x0) {
      I_5XY0(x, y);
    }
    break;

  case 0x6:
    I_6XNN(x, nn);
    break;

  case 0x7:
    I_7XNN(x, nn);
    break;

  case 0x8:
    switch (n) {
    case 0x0:
      I_8XY0(x, y);
      break;
    case 0x1:
      I_8XY1(x, y);
      break;
    case 0x2:
      I_8XY2(x, y);
      break;
    case 0x3:
      I_8XY3(x, y);
      break;
    case 0x4:
      I_8XY4(x, y);
      break;
    case 0x5:
      I_8XY5(x, y);
      break;
    case 0x6:
      I_8XY6(x, y);
      break;
    case 0x7:
      I_8XY7(x, y);
      break;
    case 0xE:
      I_8XYE(x, y);
      break;
    default:
      break;
    }
    break;

  case 0x9:
    if (n == 0x0) {
      I_9XY0(x, y);
    }
    break;

  case 0xA:
    I_ANNN(nnn);
    break;

  case 0xB:
    I_BNNN(nnn);
    break;

  case 0xC:
    I_CXNN(x, nn);
    break;

  case 0xD:
    I_DXYN(x, y, n);
    break;

  case 0xE:
    switch (nn) {
    case 0x9E:
      I_EX9E(x);
      break;
    case 0xA1:
      I_EXA1(x);
      break;
    default:
      break;
    }
    break;

  case 0xF:
    switch (nn) {
    case 0x07:
      I_FX07(x);
      break;
    case 0x0A:
      I_FX0A(x);
      break;
    case 0x15:
      I_FX15(x);
      break;
    case 0x18:
      I_FX18(x);
      break;
    case 0x1E:
      I_FX1E(x);
      break;
    case 0x29:
      I_FX29(x);
      break;
    case 0x33:
      I_FX33(x);
      break;
    case 0x55:
      I_FX55(x);
      break;
    case 0x65:
      I_FX65(x);
      break;
    default:
      break;
    }
    break;

  default:
    break;
  }
}

void Chip8::I_00E0() { m_display.fill(0); }

void Chip8::I_00EE() {
  uint16_t ret = (m_memory[m_sp] << 8) | m_memory[m_sp + 1];
  m_sp += 2;

  m_pc = ret;
}

// Deprecated
void Chip8::I_0NNN(uint16_t nnn) {
  (void)nnn;
  // Not Implemented
}

void Chip8::I_1NNN(uint16_t nnn) { m_pc = nnn; }

void Chip8::I_2NNN(uint16_t nnn) {
  m_sp -= 2;
  m_memory[m_sp] = (m_pc >> 8) & 0x00FF;
  m_memory[m_sp + 1] = m_pc & 0x00FF;

  m_pc = nnn;
}

void Chip8::I_3XNN(uint8_t x, uint8_t nn) {
  if (V[x] == nn)
    m_pc += 2;
}

void Chip8::I_4XNN(uint8_t x, uint8_t nn) {
  if (V[x] != nn)
    m_pc += 2;
}

void Chip8::I_5XY0(uint8_t x, uint8_t y) {
  if (V[x] == V[y])
    m_pc += 2;
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
    m_pc += 2;
}

void Chip8::I_ANNN(uint16_t nnn) { m_i = nnn; }

void Chip8::I_BNNN(uint16_t nnn) { m_pc = nnn + V[0]; }

void Chip8::I_CXNN(uint8_t x, uint8_t nn) { V[x] = static_cast<uint8_t>(m_distrib(m_gen) & nn); }

void Chip8::I_DXYN(uint8_t x, uint8_t y, uint8_t n) {
  uint8_t edgeH = static_cast<uint8_t>(std::min(int(V[x] % DISPLAY_WIDTH + 8), DISPLAY_WIDTH));
  uint8_t edgeV = static_cast<uint8_t>(std::min(int(V[y] % DISPLAY_HEIGHT + n), DISPLAY_HEIGHT));

  V[0xF] = 0;

  uint8_t c{0};
  for (uint8_t i = static_cast<uint8_t>(V[y] % DISPLAY_HEIGHT); i < edgeV; i++) {
    uint8_t spriteRow{m_memory[m_i + c]};

    uint8_t shiftC = 7;
    for (uint8_t j = static_cast<uint8_t>(V[x] % DISPLAY_WIDTH); j < edgeH; j++) {
      uint8_t spritePixel = (spriteRow >> shiftC) & 0x01;
      uint8_t displayPixel = m_display[static_cast<size_t>(i * DISPLAY_WIDTH + j)];

      V[0xF] |= (spritePixel && displayPixel);

      m_display[static_cast<size_t>(i * DISPLAY_WIDTH + j)] = spritePixel ^ displayPixel;
      shiftC--;
    }
    c++;
  }
}

void Chip8::I_EX9E(uint8_t x) {
  if (m_keys[V[x] & 0x0F]) {
    m_pc += 2;
  }
}

void Chip8::I_EXA1(uint8_t x) {
  if (!m_keys[V[x] & 0x0F]) {
    m_pc += 2;
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
  m_i = static_cast<uint16_t>(FONT_DATA_ADDRESS + (V[x] & 0x0F) * FONT_SPRITE_SIZE);
}

void Chip8::I_FX33(uint8_t x) {
  m_memory[m_i] = V[x] / 100;
  m_memory[m_i + 1] = (V[x] / 10) % 10;
  m_memory[m_i + 2] = V[x] % 10;
}

void Chip8::I_FX55(uint8_t x) {
  for (uint8_t i = 0; i <= x; i++) {
    m_memory[m_i + i] = V[i];
  }

  m_i = static_cast<uint16_t>(m_i + x + 1);
}

void Chip8::I_FX65(uint8_t x) {
  for (uint8_t i = 0; i <= x; i++) {
    V[i] = m_memory[m_i + i];
  }

  m_i = static_cast<uint16_t>(m_i + x + 1);
}