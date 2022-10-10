#pragma once
#include <iostream>


bool AudioMixer_Init();

void AudioMixer_PlaySound(std::string path, int audio, bool loop = false);


void AudioMixer_PlayMusic(std::string path, bool loop);

void AudioMixer_PlayMusicLoop(std::string path);
void AudioMixer_PlayMusicUnique(std::string path);

void AudioMixer_PlayMusicAsync(std::string path, bool loop = false);