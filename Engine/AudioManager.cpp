#include "AudioManager.h"
#include <fstream>
AudioManager::AudioManager() {
	
}
AudioManager::~AudioManager() {}
bool fileExists(const std::string& filename) {
	std::ifstream file(filename);
	return file.good();
}
void AudioManager::play(const std::string& filename, const bool& looped) {
	Log(fileExists("Resources/"+filename)?"File found":"File not found");
	
}