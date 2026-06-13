#include <SDL3/SDL_audio.h>
#include <cstddef>
#include <SDL3/SDL.h>
#include <memory>
#include "audio.hpp"
#include "state.hpp"
#include "main.hpp"

namespace Audio
{

constexpr SDL_AudioSpec AudioSpec = 
{
    .format = SDL_AUDIO_F32,
    .channels = 2,
    .freq = 44100
};

static void handle_audio_stream_get(void* userdata, SDL_AudioStream* stream, int additional_amount, [[maybe_unused]] int total_amount)
{
    auto state = static_cast<state_t*>(userdata);
    const auto count = static_cast<std::size_t>(additional_amount) / sizeof(float);

    if (state->handle_audio_stream_get_callback != nullptr)
    {
        auto buffer = std::make_unique<float[]>(count);

        state->handle_audio_stream_get_callback(buffer.get(), static_cast<std::uint32_t>(count));

	    if (!SDL_PutAudioStreamData(stream, buffer.get(), additional_amount))
            Main::throw_sdl_error("Couldn't put data into audio stream");
    }
    else
    {
        SDL_PauseAudioStreamDevice(state->audio_stream);
    }
}

void initialize(state_t* state)
{
    state->audio_device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &AudioSpec);

    if (state->audio_device_id == 0)
        Main::throw_sdl_error("Failed to open audio device");

    state->audio_stream = SDL_CreateAudioStream(&AudioSpec, nullptr);

    if (state->audio_stream == nullptr)
        Main::throw_sdl_error("Failed to create audio stream");

    if (!SDL_SetAudioStreamGetCallback(state->audio_stream, handle_audio_stream_get, state))
        Main::throw_sdl_error("Failed to set audio stream get callback");

    if (!SDL_BindAudioStream(state->audio_device_id, state->audio_stream))
        Main::throw_sdl_error("Failed to bind stream to audio device");
}

void destroy(state_t* state)
{
    SDL_UnbindAudioStream(state->audio_stream);
    SDL_DestroyAudioStream(state->audio_stream);
    SDL_CloseAudioDevice(state->audio_device_id);
}

void set_handle_audio_stream_get_callback(state_t* state, handle_audio_stream_get_fn_t callback)
{
    state->handle_audio_stream_get_callback = callback;
    
    if (!SDL_ResumeAudioStreamDevice(state->audio_stream))
        Main::throw_sdl_error("Failed to resume SDL audio device stream");
}

}