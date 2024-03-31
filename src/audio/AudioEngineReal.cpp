#include "AudioEngine.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include "bzip2/bz2wrap.h"

#include "elements.wav.bz2.h"

#include "Sound.h"

struct SDLData
{
	SDL_AudioSpec spec;
	SDL_AudioDeviceID device;
};

AudioEngine::AudioEngine() :
	ready(false),
	sdlData(std::make_unique<SDLData>()),
	sounds({ nullptr }),
	counts({ 0 }),
	playing(0),
	buf(nullptr)
{
	std::vector<char> elements_wav;
	if (BZ2WDecompress(elements_wav, reinterpret_cast<const char *>(elements_wav_bz2), elements_wav_bz2_size) != BZ2WDecompressOk)
	{
		std::cerr << "AudioEngine::AudioEngine->BZ2WDecompress" << std::endl;
		return;
	}

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		std::cerr << "AudioEngine::AudioEngine->SDL_InitSubSystem: " << SDL_GetError() << std::endl;
		return;
	}

	if (!SDL_LoadWAV_RW(SDL_RWFromConstMem(elements_wav.data(), elements_wav.size()), 1, &sdlData->spec, reinterpret_cast<uint8_t**>(&audio_buf), &audio_len))
	{
		std::cerr << "AudioEngine::AudioEngine->SDL_LoadWAV_RW: " << SDL_GetError() << std::endl;
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return;
	}
	audio_len /= 2; // 8 to 16 bit conversion

	sdlData->spec.callback = &SDL_AudioCallback;
	sdlData->spec.userdata = this;

	sdlData->device = SDL_OpenAudioDevice(NULL, 0, &sdlData->spec, NULL, 0);
	if (!sdlData->device)
	{
		std::cerr << "AudioEngine::AudioEngine->SDL_OpenAudioDevice: " << SDL_GetError() << std::endl;
		SDL_FreeWAV(reinterpret_cast<uint8_t*>(audio_buf));
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return;
	}

	ready = true;
}

void AudioEngine::SDL_AudioCallback(void* userdata, uint8_t* stream, int len)
{
	auto& eng = *reinterpret_cast<AudioEngine*>(userdata);

	if (!eng.buf)
	{
		eng.buf = std::make_unique<float[]>(len / 2);
	}
	std::fill_n(eng.buf.get(), len / 2, 0.0f);

	for (auto& i : eng.sounds)
	{
		if (i)
		{
			if (i->HasSamples())
			{
				i->AddSamples(eng.buf.get(), len / 2);
			}
			if (!i->HasSamples())
			{
				eng.counts[i->GetIndex()]--;
				eng.playing--;
				i = nullptr;
			}
		}
	}

	for (int i = 0; i < len / 2; i++)
	{
		reinterpret_cast<int16_t*>(stream)[i] = static_cast<int16_t>(std::tanh(eng.buf[i]) * static_cast<float>(SDL_MAX_SINT16));
	}

	if (!eng.playing)
	{
		SDL_PauseAudioDevice(eng.sdlData->device, 1);
	}
}

int AudioEngine::SoundsPlaying()
{
	return playing;
}

int AudioEngine::GetMaxSounds()
{
	return maxSounds;
}

void AudioEngine::SetMaxSounds(int maxSounds)
{
	this->maxSounds = maxSounds;
}

void AudioEngine::Play(int index)
{
	if (ready && index >= 0 && index <= 199 && (!counts[index] || index > 193) && (playing < maxSounds))
	{
		SDL_LockAudioDevice(sdlData->device);
		for (auto& i : sounds)
		{
			if (!i)
			{
				i = std::make_unique<Sound>(audio_buf, index);
				counts[index]++;
				playing++;
				SDL_PauseAudioDevice(sdlData->device, 0);
				break;
			}
		}
		SDL_UnlockAudioDevice(sdlData->device);
	}
}

AudioEngine::~AudioEngine()
{
	if (ready)
	{
		SDL_CloseAudioDevice(sdlData->device);
		SDL_FreeWAV(reinterpret_cast<uint8_t*>(audio_buf));
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
}
