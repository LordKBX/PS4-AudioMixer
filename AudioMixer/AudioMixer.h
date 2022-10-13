/*
PS4 Orbis WAV audio mixer. Choice of public domain or MIT-0. See license statements at the end of this file.
AudioMixer - v1.0.0 - 2022-10-10

Author : LordKBX

GitHub: https://github.com/LordKBX/PS4-AudioMixer
*/
#pragma once

#ifndef LKBX_AUDIOMIXER
#define LKBX_AUDIOMIXER 1
#include <iostream>
#include <vector>
#include <orbis/AudioOut.h>
#include <orbis/UserService.h>
#include <thread>         // std::thread
#include <mutex>         // std::thread

#include <algorithm>
#include <list>
#include <map>
#include <iterator>

//typedef   signed short          drwav_int16;

class SoundStruct
{
public:
	size_t sampleCount;
	signed short* pSampleData;
	SoundStruct(size_t sampleCount) {
		this->sampleCount = sampleCount;
		this->pSampleData = (signed short*)malloc(sampleCount * sizeof(uint16_t));
	}
};

class AudioMixer {
public:
	AudioMixer();
	AudioMixer(std::string base_folder);
	bool IsInitialized() { return initialized; };
	void PlaySound(std::string path);
	void PlayMusic(std::string path);// music looped
	void PlayLoop(std::string path);
	void PlayUnique(std::string path);
	std::vector<std::string> List_Availlable();
	std::vector<std::string> List_Cached();
private:
	int32_t serviceId;
	std::vector<std::thread> list_thread_sound;
	std::vector<std::thread> list_thread_music;
	std::vector<int> list_musics_channel;
	std::map<std::string, SoundStruct> cache_sounds;
	std::mutex cache_mutex;
	bool initialized = false;
	std::string base_folder = "";
	void ReadSound(std::string path, bool loop = false);
};
#endif