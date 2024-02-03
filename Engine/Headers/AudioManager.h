#pragma once
#ifndef _AUDIOMANAGER_H
#define _AUDIOMANAGER_H

#include "Engine.h"

#include "portaudio.h"
#include <sndfile.hh>

class Sound;
class AudioManager {
	public:
	bool initialized=false;
	std::vector<Sound *> sounds;
	AudioManager();
	~AudioManager();
	Sound *createSound(const std::string &soundFile, const unsigned int &volume=100u, const bool &loop=false);

	static int GetNumDevices();
	static const PaDeviceInfo *GetActiveDeviceInfo();
	static void PrintActiveDevice();
	static void PrintDevices();
};
typedef std::function<void()> voidfunc;
class Sound {
	public:
	SNDFILE *sndFile=nullptr;
	SF_INFO sfInfo;
	PaStream *stream;
	AudioManager *manager;
	std::string filePath;
	bool loop;
	int volume;
	bool playing=false;
	bool ended=false;
	float pitch=1.0f;
	voidfunc onEnd;

	static int SoundPaStreamCallback(
		const void *inputBuffer,
		void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo *timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);

	Sound(AudioManager *_manager, const std::string &soundFile, const unsigned int &_volume=100u, const bool &_loop=false);
	~Sound();
	void Play();
	void Pause();
};

#endif// _AUDIOMANAGER_H