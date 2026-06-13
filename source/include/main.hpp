#pragma once

#include <string>
#include <fstream>
#include <vector>
#include "types.h"
#include "state.hpp"

namespace Main
{

constexpr auto AppName = "Wipeout";
constexpr auto AppVersion = "1.0";
constexpr auto AppIdentifier = "dev.wipeout";
constexpr auto WindowName = "wipEout";
constexpr auto DefaultWindowWidth = 1280;
constexpr auto DefaultWindowHeight = 720;

void initialize(state_t* state);
void destroy(state_t* state);

void throw_sdl_error(const std::string& message);

}

std::ifstream platform_open_asset(const char *name);
std::vector<char> platform_load_asset(const char *name);
std::vector<char> platform_load_userdata(const char *name);

#ifdef __cplusplus
extern "C" {
#endif

void platform_exit(void);
vec2i_t platform_screen_size(void);
double platform_now(void);
bool platform_get_fullscreen(void);
void platform_set_fullscreen(bool fullscreen);
void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len));
uint32_t platform_store_userdata(const char *name, void *bytes, int32_t len);

#ifdef __cplusplus
}
#endif