#pragma once

#include "state.hpp"

namespace Audio
{

void initialize(state_t* state);
void destroy(state_t* state);

void set_handle_audio_stream_get_callback(state_t* state, handle_audio_stream_get_fn_t callback);

}
