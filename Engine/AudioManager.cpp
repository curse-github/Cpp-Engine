#include "AudioManager.h"
#include <fstream>
bool fileExists(const std::string& filename) {
	std::ifstream file(filename);
	return file.good();
}

AudioManager::AudioManager() {}
AudioManager::~AudioManager() {}
void AudioManager::play(const std::string& filename, const bool& looped) {
	Log(fileExists("Resources/"+filename) ? "File found" : "File not found");
}