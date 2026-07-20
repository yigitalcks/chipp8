#include "emulator.h"
#include "SDL3/SDL_render.h"

Emulator::Emulator() {
	SDL_SetAppMetadata("Chipp8", "0.5", "zort");

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return;
	}

	if (!SDL_CreateWindowAndRenderer("Chipp8", constants::windowWidth, constants::windowHeight, SDL_WINDOW_RESIZABLE, &m_window, &m_renderer)) {
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return;
	}
	SDL_SetRenderVSync(m_renderer, 1);

	SDL_SetRenderLogicalPresentation(m_renderer, constants::windowWidth, constants::windowHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	initAudio();
}

static void SDLCALL audioStreamCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
	(void)userdata;
	(void)total_amount;
	if (additional_amount > 0) {
		int sampleRate = 44100;
		int toneFreq = 440;
		int period = sampleRate / toneFreq;
		static int phase = 0;

		std::vector<int8_t> buffer(static_cast<size_t>(additional_amount));
		for (int i = 0; i < additional_amount; ++i) {
			buffer[static_cast<size_t>(i)] = ((phase % period) < (period / 2)) ? 30 : -30;
			phase = (phase + 1) % period;
		}
		SDL_PutAudioStreamData(stream, buffer.data(), additional_amount);
	}
}

void Emulator::initAudio() {
	SDL_AudioSpec spec;
	spec.format = SDL_AUDIO_S8;
	spec.channels = 1;
	spec.freq = 44100;

	m_audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
																						&spec, nullptr, nullptr);
	if (m_audioStream) {
		SDL_SetAudioStreamGetCallback(m_audioStream, audioStreamCallback, nullptr);
		SDL_PauseAudioStreamDevice(m_audioStream);
	}
}

Emulator::~Emulator() {
	if (m_audioStream) {
		SDL_DestroyAudioStream(m_audioStream);
		m_audioStream = nullptr;
	}
	if (m_window) {
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}
	if (m_renderer) {
		SDL_DestroyRenderer(m_renderer);
		m_renderer = nullptr;
	}
	SDL_Quit();
}

bool Emulator::load(const std::filesystem::path &path) {
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		std::cerr << "Loading Rom failed: " << path << "\n";
		return false;
	}

	size_t size = std::filesystem::file_size(path);
	if (size > PROGRAM_SIZE_LIMIT) {
		std::cerr << "Rom size limit has been exceeded! Rom size: " << size << ", limit: " << PROGRAM_SIZE_LIMIT << " bytes\n";
		return false;
	}

	// reading rom into memory address 0x200
	file.read(reinterpret_cast<char *>(&m_chip8.memory[PROGRAM_ADDRESS]), static_cast<std::streamsize>(size));

	return true;
}

uint16_t Emulator::fetch() {
	if (m_chip8.pc >= MEMORY_SIZE - 1) {
		return 0;
	}
	uint16_t instruction = (m_chip8.memory[m_chip8.pc] << 8) | m_chip8.memory[m_chip8.pc + 1];
	m_chip8.pc += 2;

	return instruction;
}

void Emulator::decode_and_execute(uint16_t instruction) {
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
			m_chip8.I_00E0();
			break;
		case 0xEE:
			m_chip8.I_00EE();
			break;
		default:
			m_chip8.I_0NNN(nnn);
			break;
		}
		break;

	case 0x1:
		m_chip8.I_1NNN(nnn);
		break;

	case 0x2:
		m_chip8.I_2NNN(nnn);
		break;

	case 0x3:
		m_chip8.I_3XNN(x, nn);
		break;

	case 0x4:
		m_chip8.I_4XNN(x, nn);
		break;

	case 0x5:
		if (n == 0x0) {
			m_chip8.I_5XY0(x, y);
		}
		break;

	case 0x6:
		m_chip8.I_6XNN(x, nn);
		break;

	case 0x7:
		m_chip8.I_7XNN(x, nn);
		break;

	case 0x8:
		switch (n) {
		case 0x0:
			m_chip8.I_8XY0(x, y);
			break;
		case 0x1:
			m_chip8.I_8XY1(x, y);
			break;
		case 0x2:
			m_chip8.I_8XY2(x, y);
			break;
		case 0x3:
			m_chip8.I_8XY3(x, y);
			break;
		case 0x4:
			m_chip8.I_8XY4(x, y);
			break;
		case 0x5:
			m_chip8.I_8XY5(x, y);
			break;
		case 0x6:
			m_chip8.I_8XY6(x, y);
			break;
		case 0x7:
			m_chip8.I_8XY7(x, y);
			break;
		case 0xE:
			m_chip8.I_8XYE(x, y);
			break;
		default:
			break;
		}
		break;

	case 0x9:
		if (n == 0x0) {
			m_chip8.I_9XY0(x, y);
		}
		break;

	case 0xA:
		m_chip8.I_ANNN(nnn);
		break;

	case 0xB:
		m_chip8.I_BNNN(nnn);
		break;

	case 0xC:
		m_chip8.I_CXNN(x, nn);
		break;

	case 0xD:
		m_chip8.I_DXYN(x, y, n);
		break;

	case 0xE:
		switch (nn) {
		case 0x9E:
			m_chip8.I_EX9E(x);
			break;
		case 0xA1:
			m_chip8.I_EXA1(x);
			break;
		default:
			break;
		}
		break;

	case 0xF:
		switch (nn) {
		case 0x07:
			m_chip8.I_FX07(x);
			break;
		case 0x0A:
			m_chip8.I_FX0A(x);
			break;
		case 0x15:
			m_chip8.I_FX15(x);
			break;
		case 0x18:
			m_chip8.I_FX18(x);
			break;
		case 0x1E:
			m_chip8.I_FX1E(x);
			break;
		case 0x29:
			m_chip8.I_FX29(x);
			break;
		case 0x33:
			m_chip8.I_FX33(x);
			break;
		case 0x55:
			m_chip8.I_FX55(x);
			break;
		case 0x65:
			m_chip8.I_FX65(x);
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

bool Emulator::run(const std::filesystem::path &path) {

	if (!m_window || !m_renderer) {
		std::cerr << "Window or renderer is not initialized!\n";
		return false;
	}

	if (!load(path)) {
		return false;
	}

	SDL_Texture *tex = SDL_CreateTexture( m_renderer, SDL_PIXELFORMAT_INDEX8, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);

	SDL_Palette *palette = SDL_CreatePalette(2);
	SDL_Color colors[2] = {{0, 0, 0, 255}, {255, 255, 255, 255}};
	SDL_SetPaletteColors(palette, colors, 0, 2);
	SDL_SetTexturePalette(tex, palette);

	const Uint64 freq = SDL_GetPerformanceFrequency();
	const Uint64 cpuTargetTicks = freq / CPU_HZ;
	const Uint64 timerTargetTicks = freq / TIMER_HZ;

	Uint64 last_time = SDL_GetPerformanceCounter();
	Uint64 cpuAccumulator = 0;
	Uint64 timerAccumulator = 0;

	bool running{true};
	while (running) {
		while (SDL_PollEvent(&m_event)) {
			if (m_event.type == SDL_EVENT_QUIT) {
				running = false;
			} else if (m_event.type == SDL_EVENT_KEY_DOWN) {
				for (const auto &[sdl_key, chip8_key] : Chip8::kKeyMap) {
					if (sdl_key == m_event.key.scancode) {
						m_chip8.setKey(chip8_key, true);
					}
				}
			} else if (m_event.type == SDL_EVENT_KEY_UP) {
				for (const auto &[sdl_key, chip8_key] : Chip8::kKeyMap) {
					if (sdl_key == m_event.key.scancode) {
						m_chip8.setKey(chip8_key, false);
					}
				}
			}
		}

		Uint64 current_time = SDL_GetPerformanceCounter();
		Uint64 frame_time = current_time - last_time;
		last_time = current_time;

		if (frame_time > freq / 4)
			frame_time = freq / 4;

		cpuAccumulator += frame_time;
		timerAccumulator += frame_time;

		while (cpuAccumulator >= cpuTargetTicks) {
			uint16_t instruction = fetch();
			decode_and_execute(instruction);
			cpuAccumulator -= cpuTargetTicks;
		}

		while (timerAccumulator >= timerTargetTicks) {
			if (m_chip8.getDelayTimer() > 0) {
				m_chip8.decreaseDelayTimer();
			}
			if (m_chip8.getSoundTimer() > 0) {
				m_chip8.decreaseSoundTimer();
			}
			timerAccumulator -= timerTargetTicks;
		}

		if (m_chip8.getSoundTimer() > 0) {
			if (m_audioStream) {
				SDL_ResumeAudioStreamDevice(m_audioStream);
			}
		} else {
			if (m_audioStream) {
				SDL_PauseAudioStreamDevice(m_audioStream);
			}
		}

		SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
		SDL_RenderClear(m_renderer);

		SDL_UpdateTexture(tex, nullptr, m_chip8.display.data(),
											DISPLAY_WIDTH *
													sizeof(decltype(m_chip8.display)::value_type));
		SDL_RenderTexture(m_renderer, tex, nullptr, nullptr);

		SDL_RenderPresent(m_renderer);
	}

	return true;
}