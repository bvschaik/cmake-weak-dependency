#ifndef SOUND_DEVICE_H
#define SOUND_DEVICE_H

enum {
    SOUND_CHANNEL_SPEECH = 0,
    SOUND_CHANNEL_MAX = SOUND_CHANNEL_SPEECH + 1
};

void sound_device_open(void);
void sound_device_close(void);

void sound_device_set_channel_volume(int channel, int volume_pct);

void sound_device_play_file_on_channel(const char *filename, int channel, int volume_pct);
void sound_device_stop_channel(int channel);

#endif // SOUND_DEVICE_H
