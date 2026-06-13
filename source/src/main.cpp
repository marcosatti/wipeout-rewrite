#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "main.hpp"
#include "state.hpp"
#include "audio.hpp"
#include "video.hpp"
#include "input.h"
#include "system.h"
#include "utils.h"

constexpr auto AppName = "Wipeout";
constexpr auto AppVersion = "1.0";
constexpr auto AppIdentifier = "dev.wipeout";

namespace Main
{

static state_t* g_state;

void throw_sdl_error(const std::string& message)
{
	auto formatted = std::format("SDL error: {}: {}", message, SDL_GetError());
	throw std::runtime_error(formatted);
}

void initialize([[maybe_unused]] state_t* state)
{
    SDL_SetAppMetadata(AppName, AppVersion, AppIdentifier);

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD)) 
		throw_sdl_error("Couldn't initialize SDL");
}

void destroy([[maybe_unused]] state_t* state)
{
	SDL_Quit();
}

}

SDL_AppResult SDL_AppInit(void **appstate, [[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	state_t* state = new state_t{};
	*appstate = state;
	Main::g_state = state;

	try
	{
		Main::initialize(state);
		Audio::initialize(state);
		Video::initialize(state);
	}
	catch (const std::runtime_error& exception)
	{
		std::cerr << exception.what() << std::endl;
		return SDL_APP_FAILURE;
	}

	system_init();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, [[maybe_unused]] SDL_AppResult result)
{
	state_t* state = static_cast<state_t*>(appstate);

	system_cleanup();

	try
	{
		Video::destroy(state);
		Audio::destroy(state);
		Main::destroy(state);
	}
	catch (const std::runtime_error& exception)
	{
		std::cerr << exception.what() << std::endl;
	}

	delete state;
	Main::g_state = nullptr;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	state_t* state = static_cast<state_t*>(appstate);
	auto& ev = *event;

	// Detect ALT+Enter press to toggle fullscreen
	if (
		ev.type == SDL_EVENT_KEY_DOWN && 
		ev.key.scancode == SDL_SCANCODE_RETURN &&
		(ev.key.mod & (SDL_KMOD_LALT | SDL_KMOD_RALT))
	) {
		platform_set_fullscreen(!platform_get_fullscreen());
	}

	// Input Keyboard
	else if (ev.type == SDL_EVENT_KEY_DOWN || ev.type == SDL_EVENT_KEY_UP) 
	{
		int code = ev.key.scancode;
		float state = ev.type == SDL_EVENT_KEY_DOWN ? 1.0 : 0.0;
		if (code >= SDL_SCANCODE_LCTRL && code <= SDL_SCANCODE_RALT) 
		{
			int code_internal = code - SDL_SCANCODE_LCTRL + INPUT_KEY_LCTRL;
			input_set_button_state(code_internal, state);
		}
		else if (code > 0 && code < INPUT_KEY_MAX) 
		{
			input_set_button_state(code, state);
		}
	}

	else if (ev.type == SDL_EVENT_TEXT_INPUT) 
	{
		input_textinput(ev.text.text[0]);
	}

	// Mouse buttons
	else if (
		ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
		ev.type == SDL_EVENT_MOUSE_BUTTON_UP
	) {
		button_t button = INPUT_BUTTON_NONE;
		
		switch (ev.button.button) {
			case SDL_BUTTON_LEFT: button = INPUT_MOUSE_LEFT; break;
			case SDL_BUTTON_MIDDLE: button = INPUT_MOUSE_MIDDLE; break;
			case SDL_BUTTON_RIGHT: button = INPUT_MOUSE_RIGHT; break;
			default: break;
		}

		if (button != INPUT_BUTTON_NONE) 
		{
			float state = ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1.0 : 0.0;
			input_set_button_state(button, state);
		}
	}

	// Mouse wheel
	else if (ev.type == SDL_EVENT_MOUSE_WHEEL) 
	{
		button_t button = ev.wheel.y > 0 
			? INPUT_MOUSE_WHEEL_UP
			: INPUT_MOUSE_WHEEL_DOWN;
		input_set_button_state(button, 1.0);
		input_set_button_state(button, 0.0);
	}

	// Mouse move
	else if (ev.type == SDL_EVENT_MOUSE_MOTION) 
	{
		input_set_mouse_pos(ev.motion.x, ev.motion.y);
	}

	// Window Events
	if (ev.type == SDL_EVENT_QUIT) 
	{
		return SDL_APP_SUCCESS;
	}
	else if (ev.type == SDL_EVENT_WINDOW_RESIZED) 
	{
		system_resize(platform_screen_size());
	}

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
	state_t* state = static_cast<state_t*>(appstate);

	system_update();
	SDL_GL_SwapWindow(state->window);

    return SDL_APP_CONTINUE;
}

void platform_exit(void) 
{
	SDL_Event event;
	SDL_zero(event);
	event.type = SDL_EVENT_QUIT;
	event.quit.timestamp = SDL_GetTicksNS();

	if (!SDL_PushEvent(&event))
		Main::throw_sdl_error("Failed to push quit event to event queue");
}

bool platform_get_fullscreen(void) 
{
	return SDL_GetWindowFlags(Main::g_state->window) & SDL_WINDOW_FULLSCREEN;
}

void platform_set_fullscreen(bool fullscreen) 
{
	if (fullscreen) 
	{
		SDL_DisplayID display = SDL_GetDisplayForWindow(Main::g_state->window);
		const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(display);
		SDL_SetWindowFullscreenMode(Main::g_state->window, mode);
		SDL_SetWindowFullscreen(Main::g_state->window, true);
		SDL_HideCursor();
	}
	else 
	{
		SDL_SetWindowFullscreen(Main::g_state->window, false);
		SDL_ShowCursor();
	}
}

void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len)) 
{
	Main::g_state->handle_audio_stream_get_callback = cb;
	
    if (!SDL_ResumeAudioStreamDevice(Main::g_state->audio_stream))
        Main::throw_sdl_error("Failed to resume SDL audio device stream");
}

std::ifstream platform_open_asset(const char *name) 
{
	auto path = ".." / std::filesystem::path{name};
	if (!std::filesystem::exists(path))
		throw std::filesystem::filesystem_error("File doesn't exist", path, std::make_error_code(std::errc::no_such_file_or_directory));
	auto stream = std::ifstream{path, std::ios_base::in | std::ios_base::binary};
	return stream;
}

std::vector<char> platform_load_asset(const char *name) 
{
	auto path = ".." / std::filesystem::path{name};
	if (!std::filesystem::exists(path))
		throw std::filesystem::filesystem_error("File doesn't exist", path, std::make_error_code(std::errc::no_such_file_or_directory));
	auto stream = std::ifstream{path, std::ios_base::in | std::ios_base::binary};
	auto vector = std::vector<char>{std::istreambuf_iterator<char>{stream}, {}};
	return vector;
}

std::vector<char> platform_load_userdata(const char *name) 
{
	auto path = "../wipeout" / std::filesystem::path{name};
	if (!std::filesystem::exists(path))
		throw std::filesystem::filesystem_error("File doesn't exist", path, std::make_error_code(std::errc::no_such_file_or_directory));
	auto stream = std::ifstream{path, std::ios_base::in | std::ios_base::binary};
	auto vector = std::vector<char>{std::istreambuf_iterator<char>{stream}, {}};
	return vector;
}

uint32_t platform_store_userdata(const char *name, void *bytes, int32_t len) 
{
	auto path = "../wipeout" / std::filesystem::path{name};

	if (std::filesystem::exists(path))
		std::filesystem::remove(path);
	
	auto stream = std::ofstream{path, std::ios_base::out | std::ios_base::binary};
	stream.write((const char *)bytes, len);
	return len;
}

vec2i_t platform_screen_size(void) {
	int width, height;
	SDL_GetWindowSizeInPixels(Main::g_state->window, &width, &height);
	return vec2i(width, height);
}

double platform_now(void)
{
	return SDL_GetTicksNS() / 1e9;
}
