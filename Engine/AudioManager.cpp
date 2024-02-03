#include "AudioManager.h"

#define Eng_Pa_Check(out) assert((out)==paNoError, "PortError: "<<Pa_GetErrorText(out));
#define BUFFER_LEN 1024

#pragma region AudioManager
AudioManager::AudioManager() {
	Eng_Pa_Check(Pa_Initialize());
	initialized=true;
}
AudioManager::~AudioManager() {
	Eng_Pa_Check(Pa_Terminate());
	while(sounds.size()>0) {
		delete sounds[0];
	}
	initialized=false;
}
int AudioManager::GetNumDevices() {
	int numDevices=Pa_GetDeviceCount();
	assert(numDevices>=0, "[AudioManager]: Pa_CountDevices returned "<<numDevices);
	return numDevices;
}
const PaDeviceInfo *AudioManager::GetActiveDeviceInfo() {
	PaDeviceIndex deviceIndex=Pa_GetDefaultOutputDevice();
	return Pa_GetDeviceInfo(deviceIndex);
}
void AudioManager::PrintActiveDevice() {
	const PaDeviceInfo *deviceInfo=GetActiveDeviceInfo();
	//int     structVersion
	//const char *    name
	//PaHostApiIndex  hostApi
	//int     maxInputChannels
	//int     maxOutputChannels
	//PaTime  defaultLowInputLatency
	//PaTime  defaultLowOutputLatency
	//PaTime  defaultHighInputLatency
	//PaTime  defaultHighOutputLatency
	//double  defaultSampleRate
	Log("structVersion: "+std::to_string(deviceInfo->structVersion));
	std::cout<<"name: "<<deviceInfo->name<<std::endl;
	Log("maxInputChannels: "+std::to_string(deviceInfo->maxInputChannels));
	Log("maxOutputChannels: "+std::to_string(deviceInfo->maxOutputChannels));
	Log("defaultSampleRate: "+std::to_string(deviceInfo->defaultSampleRate));
}
void AudioManager::PrintDevices() {
	int numDevices=GetNumDevices();
	const   PaDeviceInfo *deviceInfo;
	for(int i=0; i<numDevices; i++) {
		deviceInfo=Pa_GetDeviceInfo(i);
		Log(deviceInfo->name);
	}
}
Sound *AudioManager::createSound(const std::string &soundFile, const unsigned int &volume, const bool &loop) {
	return new Sound(this, soundFile, volume, loop);
}
#pragma endregion// AudioManager

#pragma region Sound
int Sound::SoundPaStreamCallback(
	const void *inputBuffer,
	void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo *timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData) {
	Sound *self=static_cast<Sound *>(userData);

	int channels=self->sfInfo.channels;
	unsigned int length=static_cast<unsigned int>(framesPerBuffer*self->pitch)*channels;
	float *data=new float[length];
	sf_count_t readCount=sf_read_float(self->sndFile, data, length);

	if(readCount>0u) {
		int subFormat=self->sfInfo.format&SF_FORMAT_SUBMASK;
		double scale=1.0;
		if(subFormat==SF_FORMAT_FLOAT||subFormat==SF_FORMAT_DOUBLE) {
			sf_command(self->sndFile, SFC_CALC_SIGNAL_MAX, &scale, sizeof(scale));
			if(scale<1e-10) scale=1.0;
			else scale/=32700.0;
		}
		scale*=self->volume/100.0;
		for(sf_count_t m=0;m<static_cast<unsigned int>(readCount/channels/self->pitch);m++) {
			static_cast<float *>(outputBuffer)[m]=static_cast<float>(data[((unsigned int)(m*self->pitch))*channels]*scale);
		}
	} else { // If no more data, stop the stream
		if(self->loop) {
			sf_close(self->sndFile);
			self->sndFile=sf_open(self->filePath.c_str(), SFM_READ, &self->sfInfo);
		} else {
			self->playing=false;
			delete self;
		}
	}
	delete[] data;
	return 0;
}
Sound::Sound(AudioManager *_manager, const std::string &soundFile, const unsigned int &_volume, const bool &_loop) : stream(nullptr), manager(_manager), filePath("Resources/"+soundFile), loop(_loop), volume(_volume) {
	assert(manager->initialized, "[AudioManager]: PortAudio is not setup");
	assert(!soundFile.empty(), "[AudioManager]: Invalid argument \""<<soundFile<<"\"");
	sndFile=sf_open(filePath.c_str(), SFM_READ, &sfInfo);
	assert(sndFile, "[AudioManager]: File \""<<filePath<<"\" was not found");
	const PaDeviceInfo *deviceInfo=AudioManager::GetActiveDeviceInfo();
	assert(sfInfo.channels<=deviceInfo->maxOutputChannels, "[AudioManager]: Number of channels exceeds maximum");
	// set parameters
	PaStreamParameters paStreamParameters;
	paStreamParameters.device=Pa_GetDefaultOutputDevice();
	paStreamParameters.channelCount=1;
	paStreamParameters.sampleFormat=paFloat32;
	paStreamParameters.suggestedLatency=deviceInfo->defaultLowOutputLatency;
	paStreamParameters.hostApiSpecificStreamInfo=nullptr;

	Eng_Pa_Check(Pa_OpenStream(&stream, nullptr, &paStreamParameters,
		sfInfo.samplerate, paFramesPerBufferUnspecified, paClipOff,
		SoundPaStreamCallback, this));
}
Sound::~Sound() {
	if(!ended) {
		ended=true;
		onEnd();
		Pa_CloseStream(stream);
		sf_close(sndFile); // Close the sound file
	}
	for(unsigned int i=0; i<static_cast<unsigned int>(manager->sounds.size()); i++) {
		if(manager->sounds[i]==this) manager->sounds.erase(manager->sounds.begin()+i);
	}
}
void Sound::Play() {
	Eng_Pa_Check(Pa_StartStream(stream));
	playing=true;
}
void Sound::Pause() {
	Eng_Pa_Check(Pa_StopStream(stream));
	playing=false;
}
#pragma endregion// Sound