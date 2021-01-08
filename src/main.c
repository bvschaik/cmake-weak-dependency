#include "SDL.h"
#include "SDL_mixer.h"

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>

#include <unistd.h>

#define AUDIO_RATE 22050
#define AUDIO_FORMAT AUDIO_S16
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 1024

#define EFFECT_CHANNEL 0

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Mix_Chunk *playing;
} data;

static int change_to_resources_bundle_dir(void)
{
    CFURLRef relative_url = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    CFURLRef absolute_url = CFURLCopyAbsoluteURL(relative_url);
    CFStringRef string = CFURLCopyPath(absolute_url);
    const char *resources_dir = CFStringGetCStringPtr(string, kCFStringEncodingUTF8);
    SDL_Log("Resources: %s", resources_dir);
    int change_dir_success = chdir(resources_dir) == 0;
    CFRelease(string);
    CFRelease(absolute_url);
    CFRelease(relative_url);
    return change_dir_success;
}

static void stop_sound(void)
{
    if (data.playing) {
        Mix_HaltChannel(EFFECT_CHANNEL);
        Mix_FreeChunk(data.playing);
        data.playing = 0;
    }
}

static void play_sound(const char *filename)
{
    stop_sound();
    data.playing = Mix_LoadWAV(filename);
    if (data.playing) {
        Mix_PlayChannel(EFFECT_CHANNEL, data.playing, 0);
    } else {
        SDL_Log("Unable to play file %s: %s", filename, Mix_GetError());
    }
}

static void main_loop(void)
{
    int quit = 0;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = 1;
                    } else if (event.key.keysym.sym == SDLK_x) {
                        play_sound("click.wav");
                    } else {
                        play_sound("click.mp3");
                    }
                    break;
                case SDL_QUIT:
                    quit = 1;
                    break;
            }
        }
        if (!quit) {
            SDL_RenderClear(data.renderer);
            SDL_RenderPresent(data.renderer);
        }
    }
}

static void setup(void)
{
    if (0 != SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        SDL_Log("Could not initialize SDL: %s", SDL_GetError());
        exit(-1);
    }
    if (0 != SDL_CreateWindowAndRenderer(640, 480, 0, &data.window, &data.renderer)) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        exit(-2);
    }
    if (0 != Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS)) {
        SDL_Log("Could not initialize sound: %s", Mix_GetError());
        exit(-3);
    }
    change_to_resources_bundle_dir();
}

static void teardown(void)
{
    stop_sound();
    Mix_CloseAudio();
    if (data.renderer) {
        SDL_DestroyRenderer(data.renderer);
        data.renderer = 0;
    }
    if (data.window) {
        SDL_DestroyWindow(data.window);
        data.window = 0;
    }
    SDL_Quit();
}

int main(int argc, char **argv)
{
    setup();
    main_loop();
    teardown();
    return 0;
}
