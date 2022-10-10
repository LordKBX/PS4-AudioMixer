#include <sstream>
#include <iostream>
#include <orbis/libkernel.h>

#include "../../_common/log.h"
#include "AudioMixer.h"

// Logging
std::stringstream debugLogStream;

int main(void)
{
    int sleepSeconds = 2;
    
    // No buffering
    setvbuf(stdout, NULL, _IONBF, 0);
    
    DEBUGLOG << "Hello world! Waiting " << sleepSeconds << " seconds!";
    DEBUGLOG << "Done. Infinitely looping...";

    sceKernelUsleep(2 * 1000000);

    AudioMixer_Init();
    try {
        AudioMixer_PlayMusicAsync("/app0/assets/audio/music.wav", true);
    }
    catch (const std::exception& exc) {
    }
    catch (const char* msg) {
    }

    for (;;) {
        sceKernelUsleep(2 * 1000000);
        try {
            AudioMixer_PlayMusicAsync("/app0/assets/audio/lasergun.wav", false);
        }
        catch (const std::exception& exc) {
        }
        catch (const char* msg) {
        }
    }
}
