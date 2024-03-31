#include "AudioEngine.h"

class Sound
{

};

struct SDLData
{

};

AudioEngine::AudioEngine() :
	ready(false),
	sdlData(nullptr),
	sounds({ nullptr }),
	counts({ 0 }),
	playing(0),
	buf(nullptr)
{

}

void AudioEngine::SDL_AudioCallback(void* userdata, uint8_t* stream, int len)
{

}

int AudioEngine::SoundsPlaying()
{
	return 0;
}


int AudioEngine::GetMaxSounds()
{
	return 0;
}

void AudioEngine::SetMaxSounds(int maxSounds)
{

}

void AudioEngine::Play(int index)
{

}

AudioEngine::~AudioEngine()
{

}
