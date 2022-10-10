/*
PS4 Orbis WAV audio mixer. Choice of public domain or MIT-0. See license statements at the end of this file.
AudioMixer - v1.0.0 - 2022-10-10

Author : LordKBX

GitHub: https://github.com/LordKBX/PS4-AudioMixer
*/
#pragma once
#include <iostream>


bool AudioMixer_Init();

void AudioMixer_PlaySound(std::string path);

void AudioMixer_PlayMusic(std::string path);// music looped