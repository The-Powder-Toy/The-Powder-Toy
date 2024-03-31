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

	int maxSounds = 100;
	std::array<std::unique_ptr<Sound>, 1000> sounds;
	std::array<int, 200> counts;
	std::atomic<int> playing;

	std::unique_ptr<float[]> buf;

public:
	AudioEngine();

	static void SDL_AudioCallback(void* userdata, uint8_t* stream, int len);

	int SoundsPlaying();
	int GetMaxSounds();
	void SetMaxSounds(int maxSounds);

	void Play(int index);

	~AudioEngine();
};
