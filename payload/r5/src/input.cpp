#include "inc/include.h"

void Input::init() 
{
    reset();
}

LLVM_NOOPT void Input::reset()
{
    memset(_key_states, 0, sizeof(_key_states));
    memset(_key_states_prev, 0, sizeof(_key_states_prev));
}

LLVM_NOOPT void Input::tick_begin()
{
    // update the key states of the current tick.
    for (u16 i = 0; i < INPUT_MAX_KEYS_TO_TRACK; i++)
        _key_states[i] = RS_WAS_BUTTON_DOWN(i);
}

LLVM_NOOPT void Input::tick_end()
{
    // keep track of our previous key states.
    memcpy(_key_states_prev, _key_states, sizeof(_key_states));
}

LLVM_NOOPT bool Input::is_button_down(rs::Buttons key)
{
    return _key_states[(int)key];
}

LLVM_NOOPT bool Input::was_button_pressed(rs::Buttons key)
{
    return !_key_states_prev[(int)key] && _key_states[(int)key];
}
