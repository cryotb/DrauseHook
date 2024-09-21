#pragma once

/*
 * Primary Input Manager, wraps around the game interfaces
 * to provide easy access to input detection.
*/
#define INPUT_MAX_KEYS_TO_TRACK 255

class Input
{
public:
    void init();
    void reset();

    /*
     * Handles an Input Tick. Simply checks key states and updates our states.
     * In order for the input manager to work correctly,
     * each place where timing may be different should have its own Input Mgr.
     */
    void tick_begin();
    void tick_end();

    /*
     * Checks if the key with given ID is currently held.
    */
    bool is_button_down(rs::Buttons key);

    /*
     * CHecks if the key with given ID has been toggled.
    */
    bool was_button_pressed(rs::Buttons key);
private:
    bool _key_states[INPUT_MAX_KEYS_TO_TRACK];
    bool _key_states_prev[INPUT_MAX_KEYS_TO_TRACK];
};
