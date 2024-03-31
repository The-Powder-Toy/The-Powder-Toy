#include "Sound.h"

#include <SDL2/SDL.h>

#include "OffsetTable.h"

Sound::Sound(int16_t* audio_buf_, int index_) :
	audio_buf(audio_buf_),
	index(index_),
	pos(elements_wav_offsets[index_]),
	endPos(elements_wav_offsets[index_ + 1])
{

}

int Sound::GetIndex()
{
	return index;
}

bool Sound::HasSamples()
{
	return pos != endPos;
}

void Sound::AddSamples(float* stream, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (pos != endPos)
		{
			stream[i] += static_cast<float>(audio_buf[pos++]) / static_cast<float>(SDL_MAX_SINT16);
		}
		else
		{
			return;
		}
	}
}
