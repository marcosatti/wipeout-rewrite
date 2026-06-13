#pragma once

#include <cstdint>
#include <SDL3/SDL.h>

struct state_t;

using handle_audio_stream_get_fn_t = void(*)(float *buffer, std::uint32_t len);

struct state_t
{
	SDL_Window* window;
	SDL_GLContext gl_context;
    SDL_AudioDeviceID audio_device_id;
	SDL_AudioStream* audio_stream;
    handle_audio_stream_get_fn_t handle_audio_stream_get_callback;
};
