#ifndef EMULATOR_H
#define EMULATOR_H

#include <SDL3/SDL.h>
#include <filesystem>
#include <iostream>
#include <string>
#include "chip8.h"
#include "constants.h"

class Emulator {
private:
	static constexpr uint32_t CPU_HZ = 1000;
	static constexpr uint32_t TIMER_HZ = 60;
	static constexpr uint32_t SCREEN_HZ = 60;
	static constexpr uint64_t NS_PER_SECOND = 1000000000;	
	static constexpr uint64_t nsPerCycle = NS_PER_SECOND / CPU_HZ;
	static constexpr uint64_t nsPerTimerTick = NS_PER_SECOND / TIMER_HZ;
	static constexpr uint64_t nsPerFrame = NS_PER_SECOND / SCREEN_HZ;	

	SDL_Window *m_window{nullptr};
	SDL_Renderer *m_renderer{nullptr};
	SDL_AudioStream *m_audioStream{nullptr};
	SDL_Event m_event;
	Chip8 m_chip8{};	

	void initAudio();
public:
  	Emulator();
  	~Emulator();

	bool run(const std::filesystem::path &path);
};

#endif // EMULATOR_H
