#ifndef EMULATOR_H
#define EMULATOR_H

#include <SDL3/SDL.h>
#include <filesystem>
#include "chip8.h"
#include "constants.h"

class Emulator {
private:
	static constexpr uint32_t CPU_HZ{700};
	static constexpr uint32_t TIMER_HZ{60};

	SDL_Window *m_window{nullptr};
	SDL_Renderer *m_renderer{nullptr};
	SDL_AudioStream *m_audioStream{nullptr};
	bool m_audioPlaying{false};
	SDL_Event m_event{};
	Chip8 m_chip8{};	

	static void SDLCALL audioStreamCallback(void *userdata, SDL_AudioStream *stream, int additionalAmount, int totalAmount);
	void initAudio();
public:
  	Emulator();
  	~Emulator();
	Emulator(const Emulator&) = delete;
	Emulator& operator=(const Emulator&) = delete;
	Emulator(Emulator&&) = delete;
	Emulator& operator=(Emulator&&) = delete;

	bool run(const std::filesystem::path &path);
};

#endif // EMULATOR_H
