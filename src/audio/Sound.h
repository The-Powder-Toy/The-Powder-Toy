#pragma once

#include <cstdint>

class Sound
{
	int16_t* audio_buf;

	int index;
	uint32_t pos;
	uint32_t endPos;

public:
	Sound(int16_t* audio_buf_, int index_);

	int GetIndex();
	bool HasSamples();

	void AddSamples(float* stream, int len);
};
