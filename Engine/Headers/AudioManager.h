#pragma once
#ifndef _AUDIOMANAGER_H
#define _AUDIOMANAGER_H

#include "Engine.h"
class AudioManager {
	public:
	AudioManager();
	~AudioManager();
	void play(const std::string& filename, const bool& looped=false);
};

#endif// _AUDIOMANAGER_H