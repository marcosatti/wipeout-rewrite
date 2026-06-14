#include "video.hpp"
#include "main.hpp"
#include <SDL3/SDL_video.h>

namespace Video
{

void initialize(state_t* state)
{
    char SDL_HINT_OPENGL_ES_DRIVER_value = 1;

    if (!SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER , &SDL_HINT_OPENGL_ES_DRIVER_value))
        Main::throw_sdl_error("Failed to set SDL hint");

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES))
        Main::throw_sdl_error("Failed to set OpenGL attribute");

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))
        Main::throw_sdl_error("Failed to set OpenGL attribute");

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1))
        Main::throw_sdl_error("Failed to set OpenGL attribute");

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG))
        Main::throw_sdl_error("Failed to set OpenGL attribute");

	state->window = SDL_CreateWindow(
		Main::AppName,
		Main::DefaultWindowWidth, 
        Main::DefaultWindowHeight,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY
	);

    if (!state->window)
        Main::throw_sdl_error("Failed to create window");

    state->gl_context = SDL_GL_CreateContext(state->window);

    if (state->gl_context == nullptr)
        Main::throw_sdl_error("Failed to create OpenGL ES context");

    if (!SDL_GL_SetSwapInterval(1))
        Main::throw_sdl_error("Failed to turn on vsync");
}

void destroy(state_t* state)
{
    if (!SDL_GL_DestroyContext(state->gl_context))
        Main::throw_sdl_error("Failed to destroy OpenGL context");

    SDL_DestroyWindow(state->window);
}

}