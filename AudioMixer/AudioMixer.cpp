/*
PS4 Orbis WAV audio mixer. Choice of public domain or MIT-0. See license statements at the end of this file.
AudioMixer - v1.0.0 - 2022-10-10

Author : LordKBX

GitHub: https://github.com/LordKBX/PS4-AudioMixer
*/
#include "AudioMixer.h"

// Header library for decoding wav files
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define PARAMS16 ORBIS_AUDIO_OUT_PARAM_FORMAT_S16_STEREO

#include "../Common.h"

AudioMixer::AudioMixer() {
    sceUserServiceInitialize(NULL);
    this->serviceId = sceAudioOutInit();
    if (this->serviceId != 0) { throw std::runtime_error("[ERROR] Failed to initialize audio output\n"); return; }

    this->initialized = true;
}
AudioMixer::AudioMixer(std::string base_folder) {
    if (IsPathExist(base_folder)) { this->base_folder = base_folder; }
    sceUserServiceInitialize(NULL);
    this->serviceId = sceAudioOutInit();
    if (this->serviceId != 0) { throw std::runtime_error("[ERROR] Failed to initialize audio output\n"); return; }

    this->initialized = true;
}

std::vector<std::string> AudioMixer::List_Cached() {
    std::vector<std::string> liste = std::vector<std::string>();
    for (auto it = this->cache_sounds.begin(); it != this->cache_sounds.end(); it++) {
        liste.push_back(it->first);
    }
    return liste;
}

std::vector<std::string> AudioMixer::List_Availlable() {
    std::vector<std::string> liste = std::vector<std::string>();
    if (this->base_folder == "") { return; }
    std::vector<std::string> ll = FolderListing(this->base_folder, true, ".wav");
    return liste;
}

void AudioMixer::ReadSound(std::string path, bool loop) {
    // Decode a wav file to play
    size_t sampleCount;
    drwav_int16* pSampleData;
    if (this->initialized == false) { return; }
    this->cache_mutex.lock();
    std::map<std::string, SoundStruct>::iterator it = this->cache_sounds.find(path);
    if (it != this->cache_sounds.end()) {
        sampleCount = this->cache_sounds.at(path).sampleCount;
        pSampleData = (drwav_int16*)malloc(sampleCount * sizeof(uint16_t));
        memcpy(pSampleData, this->cache_sounds.at(path).pSampleData, sampleCount * sizeof(uint16_t));
    }
    else
    {
        drwav wav;
        if (!IsPathExist(path)) { return; }
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

        this->cache_sounds.emplace(path, SoundStruct(sampleCount));
        memcpy(this->cache_sounds.at(path).pSampleData, pSampleData, sampleCount * sizeof(uint16_t));
    }
    this->cache_mutex.unlock();

    int audio = sceAudioOutOpen(ORBIS_USER_SERVICE_USER_ID_SYSTEM, ORBIS_AUDIO_OUT_PORT_TYPE_MAIN, 0, 256, 48000, PARAMS16);
    if (audio <= 0) { throw std::runtime_error("[ERROR] Failed to open audio on main port\n"); return; }

    if (audio <= 0)
    {
        throw std::runtime_error("[ERROR] Failed to open audio on main port\n");
        return;
    }
    if (loop == true) { 
        this->list_musics_channel.push_back(audio);
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

void AudioMixer::PlayLoop(std::string path) { ReadSound(path, true); }
void AudioMixer::PlayUnique(std::string path) { ReadSound(path, false); }

void AudioMixer::PlayMusic(std::string path) {
    if (initialized == false) { return; }
    for (int i = list_thread_music.size() - 1; i >= 0; --i) {
        if (list_thread_music.at(i).joinable()) {
            //list_thread_music.at(i).~thread();
        }
        list_thread_music.erase(list_thread_music.begin() + i);
    }

    list_thread_music.push_back(std::thread(&AudioMixer::PlayLoop, this, path));
}

void AudioMixer::PlaySound(std::string path) {
    if (initialized == false) { return; }
    for (int i = list_thread_sound.size() - 1; i >= 0; --i) {
        if (!list_thread_sound.at(i).joinable()) {
            list_thread_sound.at(i).~thread();
            list_thread_sound.erase(list_thread_sound.begin() + i);
        }
    }

    list_thread_music.push_back(std::thread(&AudioMixer::PlayUnique, this, path));
}