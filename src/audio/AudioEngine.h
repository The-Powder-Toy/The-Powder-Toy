#pragma once

#include <cstdint>
#include <array>
#include <memory>
#include <atomic>

class Sound;
struct SDLData; // Renderer builds don't have access to SDL

class AudioEngine
{
	bool ready;

	std::unique_ptr<SDLData> sdlData;
	int16_t* audio_buf;
	uint32_t audio_len;

	std::array<std::unique_ptr<Sound>, 100> sounds;
	std::array<int, 200> counts;
	std::atomic<int> playing;

	std::unique_ptr<float[]> buf;

public:
	AudioEngine();

	static void SDL_AudioCallback(void* userdata, uint8_t* stream, int len);

	int SoundsPlaying();

	void Play(int index);

	~AudioEngine();
};
