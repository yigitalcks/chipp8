#include "emulator.h"
#include "SDL3/SDL_render.h"
#include <array>
#include <iostream>

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

	if(!SDL_SetDefaultTextureScaleMode(m_renderer, SDL_SCALEMODE_NEAREST)) {
		SDL_Log("Couldn't set texture scale mode: %s", SDL_GetError());
	}

	SDL_Surface *icon = SDL_LoadBMP("assets/logo.bmp");
	if (!icon) {
		icon = SDL_LoadBMP("../assets/logo.bmp");
	}
	if (!icon) {
		icon = SDL_LoadBMP("macos/logo.bmp");
	}
	if (icon) {
		SDL_SetWindowIcon(m_window, icon);
		SDL_DestroySurface(icon);
	}

	SDL_SetRenderVSync(m_renderer, 1);

	SDL_SetRenderLogicalPresentation(m_renderer, constants::windowWidth, constants::windowHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	initAudio();
}

void SDLCALL Emulator::audioStreamCallback(void *userdata, SDL_AudioStream *stream, int additionalAmount, int totalAmount) {
	(void)totalAmount;
	(void)userdata;

	constexpr int sampleRate{44100};
	constexpr int toneFrequency{440};
	std::array<int8_t, 1024> buffer{};
	static int phase{};

	while (additionalAmount > 0) {
		const int byteCount{std::min(additionalAmount, static_cast<int>(buffer.size()))};
		for (int i = 0; i < byteCount; ++i) {
			buffer[static_cast<size_t>(i)] = ((phase * 2 * toneFrequency) / sampleRate % 2 == 0) ? 30 : -30;
			phase = (phase + 1) % sampleRate;
		}

		(void)SDL_PutAudioStreamData(stream, buffer.data(), byteCount);
		additionalAmount -= byteCount;
	}
}

void Emulator::initAudio() {
	SDL_AudioSpec spec{};
	spec.format = SDL_AUDIO_S8;
	spec.channels = 1;
	spec.freq = 44100;

	m_audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audioStreamCallback, nullptr);
	if (!m_audioStream) {
		SDL_Log("Couldn't open audio device: %s", SDL_GetError());
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

bool Emulator::run(const std::filesystem::path &path) {
	if (!m_window || !m_renderer) {
		std::cerr << "Window or renderer is not initialized!\n";
		return false;
	}

	if (!m_chip8.loadROM(path)) {
		return false;
	}

	SDL_Texture *tex = SDL_CreateTexture(m_renderer, 
		SDL_PIXELFORMAT_INDEX8, 
		SDL_TEXTUREACCESS_STREAMING, 
		Chip8::DISPLAY_WIDTH, 
		Chip8::DISPLAY_HEIGHT);

	SDL_Palette *palette = SDL_CreatePalette(2);
	SDL_Color colors[2]{{0, 0, 0, 255}, {255, 255, 255, 255}};
	SDL_SetPaletteColors(palette, colors, 0, 2);
	SDL_SetTexturePalette(tex, palette);

	const Uint64 freq{SDL_GetPerformanceFrequency()};
	const Uint64 cpuTargetTicks{freq / CPU_HZ};
	const Uint64 timerTargetTicks{freq / TIMER_HZ};

	Uint64 last_time{SDL_GetPerformanceCounter()};
	Uint64 cpuAccumulator{0};
	Uint64 timerAccumulator{0};

	bool running{true};
	while (running) {
		while (SDL_PollEvent(&m_event)) {
			if (m_event.type == SDL_EVENT_QUIT) {
				running = false;
			}
			else if (m_event.type == SDL_EVENT_KEY_DOWN) {
				for (const auto &[sdl_key, chip8_key] : Chip8::kKeyMap) {
					if (sdl_key == m_event.key.scancode) {
						m_chip8.setKey(chip8_key, true);
					}
				}
			}
			else if (m_event.type == SDL_EVENT_KEY_UP) {
				for (const auto &[sdl_key, chip8_key] : Chip8::kKeyMap) {
					if (sdl_key == m_event.key.scancode) {
						m_chip8.setKey(chip8_key, false);
					}
				}
			}
		}

		Uint64 current_time{SDL_GetPerformanceCounter()};
		Uint64 frame_time{current_time - last_time};
		last_time = current_time;

		if (frame_time > freq / 4)
			frame_time = freq / 4;

		cpuAccumulator += frame_time;
		timerAccumulator += frame_time;

		while (cpuAccumulator >= cpuTargetTicks) {
			m_chip8.step();
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

		const bool shouldPlay{m_chip8.getSoundTimer() > 0};
		if (m_audioStream && shouldPlay != m_audioPlaying) {
			if (shouldPlay) {
				SDL_ResumeAudioStreamDevice(m_audioStream);
			} else {
				SDL_PauseAudioStreamDevice(m_audioStream);
			}
			m_audioPlaying = shouldPlay;
		}

		SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
		SDL_RenderClear(m_renderer);

		SDL_UpdateTexture(tex, nullptr, m_chip8.getDisplayBuffer(), Chip8::DISPLAY_WIDTH * sizeof(uint8_t));
		SDL_RenderTexture(m_renderer, tex, nullptr, nullptr);

		SDL_RenderPresent(m_renderer);
	}

	SDL_DestroyPalette(palette);
	SDL_DestroyTexture(tex);

	return true;
}
