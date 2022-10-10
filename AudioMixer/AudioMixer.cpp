/*
PS4 Orbis WAV audio mixer. Choice of public domain or MIT-0. See license statements at the end of this file.
AudioMixer - v1.0.0 - 2022-10-10

Author : LordKBX

GitHub: https://github.com/LordKBX/PS4-AudioMixer
*/
#include <orbis/AudioOut.h>
#include <orbis/UserService.h>
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <mutex>         // std::thread

#include <algorithm>
#include <list>
#include <vector>
#include <map>
#include <iterator>

// Header library for decoding wav files
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define PARAMS16 ORBIS_AUDIO_OUT_PARAM_FORMAT_S16_STEREO

class SoundStruct
{
public:
    size_t sampleCount;
    drwav_int16* pSampleData;
    SoundStruct(size_t sampleCount) {
        this->sampleCount = sampleCount;
        this->pSampleData = (drwav_int16*)malloc(sampleCount * sizeof(uint16_t));
    }
};

int32_t serviceId;

std::vector<std::thread> AudioMixer_list_thread_sound;
std::vector<std::thread> AudioMixer_list_thread_music;
std::vector<int> AudioMixer_list_musics_channel;
std::map<std::string, SoundStruct> AudioMixer_cache;
std::mutex AudioMixer_cache_mutex;
bool AudioMixer_initialized = false;

bool AudioMixer_Init() {
    if (AudioMixer_initialized == false) {
        sceUserServiceInitialize(NULL);
        serviceId = sceAudioOutInit();
        if (serviceId != 0) { throw std::runtime_error("[ERROR] Failed to initialize audio output\n"); return false; }
        AudioMixer_cache = std::map<std::string, SoundStruct>();

        AudioMixer_list_musics_channel = std::vector<int>();
        AudioMixer_list_thread_sound = std::vector<std::thread>();
        AudioMixer_list_thread_music = std::vector<std::thread>();

        AudioMixer_initialized = true;
        return true;
    }
    return false;
}

void AudioMixer_ReadSound(std::string path, bool loop = false) {
    // Decode a wav file to play
    size_t sampleCount;
    drwav_int16* pSampleData;
    AudioMixer_cache_mutex.lock();
    std::map<std::string, SoundStruct>::iterator it = AudioMixer_cache.find(path);
    if (it != AudioMixer_cache.end()) {
        sampleCount = AudioMixer_cache.at(path).sampleCount;
        pSampleData = (drwav_int16*)malloc(sampleCount * sizeof(uint16_t));
        memcpy(pSampleData, AudioMixer_cache.at(path).pSampleData, sampleCount * sizeof(uint16_t));
    }
    else
    {
        drwav wav;
        bool loaded = drwav_init_file(&wav, path.c_str(), NULL);
        if (!loaded)
        {
            throw std::runtime_error("[ERROR] Failed to decode wav file\n");
            return;
        }

        // Calculate the sample count and allocate a buffer for the sample data accordingly
        sampleCount = wav.totalPCMFrameCount * wav.channels;
        pSampleData = (drwav_int16*)malloc(sampleCount * sizeof(uint16_t));

        // Decode the wav into pSampleData
        drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pSampleData);
        drwav_uninit(&wav);

        AudioMixer_cache.emplace(path, SoundStruct(sampleCount));
        memcpy(AudioMixer_cache.at(path).pSampleData, pSampleData, sampleCount * sizeof(uint16_t));
    }
    AudioMixer_cache_mutex.unlock();

    int audio = sceAudioOutOpen(ORBIS_USER_SERVICE_USER_ID_SYSTEM, ORBIS_AUDIO_OUT_PORT_TYPE_MAIN, 0, 256, 48000, PARAMS16);
    if (audio <= 0) { throw std::runtime_error("[ERROR] Failed to open audio on main port\n"); return; }

    if (audio <= 0)
    {
        throw std::runtime_error("[ERROR] Failed to open audio on main port\n");
        return;
    }
    if (loop == true) {
        AudioMixer_list_musics_channel.push_back(audio);
    }

    // Play the sample
    int32_t sOffs = 0;
    drwav_int16* pSample = nullptr;

    bool finished = false;

    // Play the song in a loop
    while (!finished)
    {
        pSample = &pSampleData[sOffs];

        /* Output audio */
        sceAudioOutOutput(audio, NULL);	// NULL: wait for completion

        if (sceAudioOutOutput(audio, pSample) < 0) {
            throw std::runtime_error("Failed to output audio\n");
            break;
        }

        sOffs += 256 * 2;

        if (sOffs >= sampleCount) {
            if (loop == false) { sceAudioOutOutput(audio, NULL);  finished = true; }
            else { sOffs = 0; }
        }
    }
    sceAudioOutClose(audio);
    free(pSample);
    free(pSampleData);
}

void AudioMixer_PlayLoop(std::string path) { AudioMixer_ReadSound(path, true); }
void AudioMixer_PlayUnique(std::string path) { AudioMixer_ReadSound(path, false); }

void AudioMixer_PlayMusic(std::string path) {
    for (int i = AudioMixer_list_thread_music.size() - 1; i >= 0; --i) {
        if (AudioMixer_list_thread_music.at(i).joinable()) {
            AudioMixer_list_thread_music.at(i).~thread();
        }
        AudioMixer_list_thread_music.erase(AudioMixer_list_thread_music.begin() + i);
    }

    AudioMixer_list_thread_music.push_back(std::thread(AudioMixer_PlayLoop, path));
}

void AudioMixer_PlaySound(std::string path) {
    for (int i = AudioMixer_list_thread_sound.size() - 1; i >= 0; --i) {
        if (!AudioMixer_list_thread_sound.at(i).joinable()) {
            AudioMixer_list_thread_sound.at(i).~thread();
            AudioMixer_list_thread_sound.erase(AudioMixer_list_thread_sound.begin() + i);
        }
    }

    AudioMixer_list_thread_sound.push_back(std::thread(AudioMixer_PlayUnique, path));
}