#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>

#include "platform.h"
#include "input.h"
#include "system.h"
#include "utils.h"
#include "mem.h"

static uint64_t perf_freq = 0;
static bool wants_to_exit = false;
static SDL_Window *window;
static SDL_AudioStream* audio_device;
static SDL_Gamepad *gamepad;
static void (*audio_callback)(float *buffer, uint32_t len) = NULL;
static char *path_assets = "";
static char *path_userdata = "";
static char *temp_path = NULL;


uint8_t platform_sdl_gamepad_map[] = 
{
	[SDL_GAMEPAD_BUTTON_SOUTH] = INPUT_GAMEPAD_A,
	[SDL_GAMEPAD_BUTTON_EAST] = INPUT_GAMEPAD_B,
	[SDL_GAMEPAD_BUTTON_WEST] = INPUT_GAMEPAD_X,
	[SDL_GAMEPAD_BUTTON_NORTH] = INPUT_GAMEPAD_Y,
	[SDL_GAMEPAD_BUTTON_BACK] = INPUT_GAMEPAD_SELECT,
	[SDL_GAMEPAD_BUTTON_GUIDE] = INPUT_GAMEPAD_HOME,
	[SDL_GAMEPAD_BUTTON_START] = INPUT_GAMEPAD_START,
	[SDL_GAMEPAD_BUTTON_LEFT_STICK] = INPUT_GAMEPAD_L_STICK_PRESS,
	[SDL_GAMEPAD_BUTTON_RIGHT_STICK] = INPUT_GAMEPAD_R_STICK_PRESS,
	[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER] = INPUT_GAMEPAD_L_SHOULDER,
	[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER] = INPUT_GAMEPAD_R_SHOULDER,
	[SDL_GAMEPAD_BUTTON_DPAD_UP] = INPUT_GAMEPAD_DPAD_UP,
	[SDL_GAMEPAD_BUTTON_DPAD_DOWN] = INPUT_GAMEPAD_DPAD_DOWN,
	[SDL_GAMEPAD_BUTTON_DPAD_LEFT] = INPUT_GAMEPAD_DPAD_LEFT,
	[SDL_GAMEPAD_BUTTON_DPAD_RIGHT] = INPUT_GAMEPAD_DPAD_RIGHT,
	[SDL_GAMEPAD_BUTTON_COUNT] = INPUT_INVALID
};


uint8_t platform_sdl_axis_map[] = 
{
	[SDL_GAMEPAD_AXIS_LEFTX] = INPUT_GAMEPAD_L_STICK_LEFT,
	[SDL_GAMEPAD_AXIS_LEFTY] = INPUT_GAMEPAD_L_STICK_UP,
	[SDL_GAMEPAD_AXIS_RIGHTX] = INPUT_GAMEPAD_R_STICK_LEFT,
	[SDL_GAMEPAD_AXIS_RIGHTY] = INPUT_GAMEPAD_R_STICK_UP,
	[SDL_GAMEPAD_AXIS_LEFT_TRIGGER] = INPUT_GAMEPAD_L_TRIGGER,
	[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER] = INPUT_GAMEPAD_R_TRIGGER,
	[SDL_GAMEPAD_AXIS_COUNT] = INPUT_INVALID
};


void platform_exit(void) 
{
	wants_to_exit = true;
}

SDL_Gamepad *platform_find_gamepad(void) 
{
	int count = 0;
	SDL_Gamepad* gamepad = NULL;
	SDL_JoystickID *joysticks = SDL_GetJoysticks(&count);

	if (joysticks == NULL)
		return NULL;

	for (int i = 0; i < count; i++) 
	{
		if (SDL_IsGamepad(joysticks[i]))
		{
			gamepad = SDL_OpenGamepad(joysticks[i]);
			break;
		}
	}

	SDL_free(joysticks);
	return gamepad;
}

void platform_pump_events(void) {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
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

		// Gamepads connect/disconnect
		else if (ev.type == SDL_EVENT_GAMEPAD_ADDED) 
		{
			gamepad = SDL_OpenGamepad(ev.gdevice.which);
		}
		else if (ev.type == SDL_EVENT_GAMEPAD_REMOVED) 
		{
			if (gamepad)
			{
				SDL_Joystick* joystick = SDL_GetGamepadJoystick(gamepad);
				
				if (ev.gdevice.which == SDL_GetJoystickID(joystick))
				{
					SDL_CloseGamepad(gamepad);
					gamepad = platform_find_gamepad();
				}
			}
		}

		// Input Gamepad Buttons
		else if (
			ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || 
			ev.type == SDL_EVENT_GAMEPAD_BUTTON_UP
		) {
			if (ev.gbutton.button < SDL_GAMEPAD_BUTTON_COUNT) 
			{
				button_t button = platform_sdl_gamepad_map[ev.gbutton.button];

				if (button != INPUT_INVALID) 
				{
					float state = ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ? 1.0 : 0.0;
					input_set_button_state(button, state);
				}
			}
		}

		// Input Gamepad Axis
		else if (ev.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) 
		{
			float state = (float)ev.gaxis.value / 32767.0;

			if (ev.gaxis.axis < SDL_GAMEPAD_AXIS_COUNT) 
			{
				int code = platform_sdl_axis_map[ev.gaxis.axis];
				if (
					code == INPUT_GAMEPAD_L_TRIGGER || 
					code == INPUT_GAMEPAD_R_TRIGGER
				) {
					input_set_button_state(code, state);
				}
				else if (state > 0) 
				{
					input_set_button_state(code, 0.0);
					input_set_button_state(code+1, state);
				}
				else 
				{
					input_set_button_state(code, -state);
					input_set_button_state(code+1, 0.0);
				}
			}
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
			wants_to_exit = true;
		}
		else if (ev.type == SDL_EVENT_WINDOW_RESIZED) 
		{
			system_resize(platform_screen_size());
		}
	}
}

double platform_now(void) 
{
	uint64_t perf_counter = SDL_GetPerformanceCounter();
	return (double)perf_counter / (double)perf_freq;
}

bool platform_get_fullscreen(void) 
{
	return SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
}

void platform_set_fullscreen(bool fullscreen) 
{
	if (fullscreen) 
	{
		SDL_DisplayID display = SDL_GetDisplayForWindow(window);
		const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(display);
		SDL_SetWindowFullscreenMode(window, mode);
		SDL_SetWindowFullscreen(window, true);
		SDL_HideCursor();
	}
	else 
	{
		SDL_SetWindowFullscreen(window, false);
		SDL_ShowCursor();
	}
}

void platform_audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) 
{
	static float buffer[4096];

	if (audio_callback)
		audio_callback(buffer, sizeof(buffer) / sizeof(float));
	else
		memset(buffer, 0, sizeof(buffer));

	SDL_PutAudioStreamData(stream, buffer, sizeof(buffer));
}

void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len)) 
{
	audio_callback = cb;
	SDL_ResumeAudioStreamDevice(audio_device);
}


FILE *platform_open_asset(const char *name, const char *mode) {
	char *path = strcat(strcpy(temp_path, path_assets), name);
	return fopen(path, mode);
}

uint8_t *platform_load_asset(const char *name, uint32_t *bytes_read) {
	char *path = strcat(strcpy(temp_path, path_assets), name);
	return file_load(path, bytes_read);
}

uint8_t *platform_load_userdata(const char *name, uint32_t *bytes_read) {
	char *path = strcat(strcpy(temp_path, path_userdata), name);
	if (!file_exists(path)) {
		*bytes_read = 0;
		return NULL;
	}
	return file_load(path, bytes_read);
}

uint32_t platform_store_userdata(const char *name, void *bytes, int32_t len) {
	char *path = strcat(strcpy(temp_path, path_userdata), name);
	return file_store(path, bytes, len);
}

SDL_GLContext platform_gl;

vec2i_t platform_screen_size(void) {
	int width, height;
	SDL_GetWindowSizeInPixels(window, &width, &height);
	return vec2i(width, height);
}

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD);

	// Figure out the absolute asset and userdata paths. These may either be
	// supplied at build time through -DPATH_ASSETS=.. and -DPATH_USERDATA=..
	// or received at runtime from SDL. Note that SDL may return NULL for these.
	// We fall back to the current directory (i.e. just "") in this case.

	path_assets = "../"; 

	char *sdl_path_userdata = NULL;
	#ifdef PATH_USERDATA
		path_userdata = TOSTRING(PATH_USERDATA);
	#else
		sdl_path_userdata = SDL_GetPrefPath("phoboslab", "wipeout");
		if (sdl_path_userdata) {
			path_userdata = sdl_path_userdata;
		}
	#endif

	// Reserve some space for concatenating the asset and userdata paths with
	// local filenames.
	temp_path = mem_bump(max(strlen(path_assets), strlen(path_userdata)) + 64);

	// Load gamecontrollerdb.txt if present.
	// FIXME: Should this load from userdata instead?
	if (false)
	{
		char *gcdb_path = strcat(strcpy(temp_path, path_assets), "gamecontrollerdb.txt");
		int gcdb_res = SDL_AddGamepadMappingsFromFile(gcdb_path);

		if (gcdb_res < 0)
			printf("Failed to load gamecontrollerdb.txt\n");
		else
			printf("load gamecontrollerdb.txt\n");
	}

	gamepad = platform_find_gamepad();

	perf_freq = SDL_GetPerformanceFrequency();

	SDL_AudioSpec desired_spec = 
	{
		.channels = 2,
		.freq = 44100,
		.format = SDL_AUDIO_F32
	};

	audio_device = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec, platform_audio_callback, NULL);

	window = SDL_CreateWindow(
		SYSTEM_WINDOW_NAME,
		SYSTEM_WINDOW_WIDTH, SYSTEM_WINDOW_HEIGHT,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY
	);

#if defined(USE_GLES2)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

	platform_gl = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);

	system_init();

	while (!wants_to_exit) {
		platform_pump_events();
		system_update();
		SDL_GL_SwapWindow(window);
	}

	system_cleanup();
	SDL_GL_DestroyContext(platform_gl);

	SDL_DestroyWindow(window);

	if (gamepad) {
		SDL_CloseGamepad(gamepad);
	}

	if (sdl_path_userdata) {
		SDL_free(sdl_path_userdata);
	}

	SDL_DestroyAudioStream(audio_device);
	SDL_Quit();
	return 0;
}
