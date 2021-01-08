#include "SDL.h"

#include "sound_device.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} SDL;

int platform_screen_create(void)
{
    Uint32 flags = SDL_WINDOW_RESIZABLE;
    if (SDL_CreateWindowAndRenderer(640, 480, 0, &SDL.window, &SDL.renderer) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create window: %s", SDL_GetError());
        return 0;
    }
    return 1;
}

void platform_screen_destroy(void)
{
    if (SDL.renderer) {
        SDL_DestroyRenderer(SDL.renderer);
        SDL.renderer = 0;
    }
    if (SDL.window) {
        SDL_DestroyWindow(SDL.window);
        SDL.window = 0;
    }
}

void platform_screen_render(void)
{
    SDL_RenderClear(SDL.renderer);
    SDL_RenderPresent(SDL.renderer);
}

static void main_loop(void)
{
    platform_screen_render();
    int quit = 0;
    while (!quit) {
        SDL_Event event;
        /* Process event queue */
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = 1;
                    } else {
                        sound_device_play_file_on_channel("click.mp3", SOUND_CHANNEL_SPEECH, 100);
                    }
                    break;
                case SDL_QUIT:
                    quit = 1;
                    break;
            }
        }
        if (!quit) {
            platform_screen_render();
        }
    }
}

static int init_sdl(void)
{
    SDL_Log("Initializing SDL");
    Uint32 SDL_flags = SDL_INIT_AUDIO | SDL_INIT_VIDEO;

    if (SDL_Init(SDL_flags) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL: %s", SDL_GetError());
        return 0;
    }
    SDL_Log("SDL initialized");
    return 1;
}

static int pre_init(const char *custom_data_dir)
{
    if (custom_data_dir) {
        SDL_Log("Loading game from %s", custom_data_dir);
        if (chdir(custom_data_dir) != 0) {
            SDL_Log("%s: directory not found", custom_data_dir);
            return 0;
        }
        return 1;
    }
    return 1;
}

static void setup(const char *datadir)
{
    if (!init_sdl()) {
        SDL_Log("Exiting: SDL init failed");
        exit(-1);
    }

    if (!pre_init(datadir)) {
        SDL_Log("Exiting: game pre-init failed");
        exit(1);
    }

    if (!platform_screen_create()) {
        SDL_Log("Exiting: SDL create window failed");
        exit(-2);
    }

    sound_device_open();
    sound_device_set_channel_volume(SOUND_CHANNEL_SPEECH, 100);
}

static void teardown(void)
{
    SDL_Log("Exiting game");
    sound_device_close();
    platform_screen_destroy();
    SDL_Quit();
}

int main(int argc, char **argv)
{
    const char *datadir = 0;
    if (argc > 1) {
        datadir = argv[argc-1];
    }
    setup(datadir);

    main_loop();

    teardown();
    return 0;
}
