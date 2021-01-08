#include "sound_device.h"

#include "SDL.h"
#include "SDL_mixer.h"

#include <stdlib.h>
#include <string.h>

#define AUDIO_RATE 22050
#define AUDIO_FORMAT AUDIO_S16
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 1024

#define MAX_CHANNELS 2

typedef struct {
    Mix_Chunk *chunk;
} sound_channel;

static struct {
    int initialized;
    sound_channel channels[MAX_CHANNELS];
} data;

static int percentage_to_volume(int percentage)
{
    return percentage * SDL_MIX_MAXVOLUME / 100;
}

static void init_channels(void)
{
    data.initialized = 1;
    for (int i = 0; i < MAX_CHANNELS; i++) {
        data.channels[i].chunk = 0;
    }
}

void sound_device_open(void)
{
    if (0 == Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS)) {
        init_channels();
        return;
    }
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound failed to initialize using default driver: %s", Mix_GetError());
    // Try to work around SDL choosing the wrong driver on Windows sometimes
    for (int i = 0; i < SDL_GetNumAudioDrivers(); i++) {
        const char *driver_name = SDL_GetAudioDriver(i);
        if (SDL_strcmp(driver_name, "disk") == 0 || SDL_strcmp(driver_name, "dummy") == 0) {
            // Skip "write-to-disk" and dummy drivers
            continue;
        }
        if (0 == SDL_AudioInit(driver_name) &&
            0 == Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS)) {
            SDL_Log("Using audio driver: %s", driver_name);
            init_channels();
            return;
        } else {
            SDL_Log("Not using audio driver %s, reason: %s", driver_name, SDL_GetError());
        }
    }
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound failed to initialize: %s", Mix_GetError());
    int max = SDL_GetNumAudioDevices(0);
    SDL_Log("Number of audio devices: %d", max);
    for (int i = 0; i < max; i++) {
        SDL_Log("Audio device: %s", SDL_GetAudioDeviceName(i, 0));
    }
}

void sound_device_close(void)
{
    if (data.initialized) {
        for (int i = 0; i < MAX_CHANNELS; i++) {
            sound_device_stop_channel(i);
        }
        Mix_CloseAudio();
        data.initialized = 0;
    }
}

static Mix_Chunk *load_chunk(const char *filename)
{
    if (filename[0]) {
        return Mix_LoadWAV(filename);
    } else {
        return NULL;
    }
}

void sound_device_set_channel_volume(int channel, int volume_pct)
{
    if (data.channels[channel].chunk) {
        Mix_VolumeChunk(data.channels[channel].chunk, percentage_to_volume(volume_pct));
    }
}

void sound_device_play_file_on_channel(const char *filename, int channel, int volume_pct)
{
    if (data.initialized) {
        sound_device_stop_channel(channel);
        data.channels[channel].chunk = load_chunk(filename);
        if (data.channels[channel].chunk) {
            sound_device_set_channel_volume(channel, volume_pct);
            Mix_PlayChannel(channel, data.channels[channel].chunk, 0);
        }
    }
}

void sound_device_stop_channel(int channel)
{
    if (data.initialized) {
        sound_channel *ch = &data.channels[channel];
        if (ch->chunk) {
            Mix_HaltChannel(channel);
            Mix_FreeChunk(ch->chunk);
            ch->chunk = 0;
        }
    }
}
