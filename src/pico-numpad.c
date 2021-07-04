/* 
 * Pico SDVX
 * @author SpeedyPotato
 * 
 * Based off dev_hid_composite and mdxtinkernick/pico_encoders
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"

#include "usb_descriptors.h"

#define SW_GPIO_SIZE 22 // Number of switches

// MODIFY KEYBINDS HERE, MAKE SURE LENGTH MATCHES SW_GPIO_SIZE
const uint8_t SW_KEYCODE[] =
    {HID_KEY_KEYPAD_0, HID_KEY_KEYPAD_DECIMAL, HID_KEY_KEYPAD_ENTER, HID_KEY_KEYPAD_1, HID_KEY_KEYPAD_2, HID_KEY_KEYPAD_3, HID_KEY_KEYPAD_4, HID_KEY_KEYPAD_5, HID_KEY_KEYPAD_6, HID_KEY_KEYPAD_ADD, HID_KEY_KEYPAD_7, HID_KEY_KEYPAD_8, HID_KEY_KEYPAD_9, HID_KEY_NUM_LOCK, HID_KEY_KEYPAD_DIVIDE, HID_KEY_KEYPAD_MULTIPLY, HID_KEY_KEYPAD_SUBTRACT, HID_KEY_F1, HID_KEY_F2, HID_KEY_F3, HID_KEY_BACKSPACE};
const uint8_t SW_GPIO[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21}; // MAKE SURE LENGTH MATCHES SW_GPIO_SIZE

bool sw_val[SW_GPIO_SIZE];
bool prev_sw_val[SW_GPIO_SIZE];
bool sw_changed;

/**
 * Initialize Board Pins
 **/
void init()
{
    // Setup LED pin for numlock
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    // Setup Button GPIO
    for (int i = 0; i < SW_GPIO_SIZE; i++)
    {
        sw_val[i] = false;
        prev_sw_val[i] = false;
        gpio_init(SW_GPIO[i]);
        gpio_set_function(SW_GPIO[i], GPIO_FUNC_SIO);
        gpio_set_dir(SW_GPIO[i], GPIO_IN);
        gpio_pull_up(SW_GPIO[i]);
    }

    // Set listener bools
    sw_changed = false;
}

/**
 * Update Class Vars
 **/
void update_inputs()
{
    // Switch Update & Flag
    for (int i = 0; i < SW_GPIO_SIZE; i++)
    {
        if (!gpio_get(SW_GPIO[i]))
        {
            sw_val[i] = true;
        }
        else
        {
            sw_val[i] = false;
        }
        if (!sw_changed && sw_val[i] != prev_sw_val[i])
        {
            sw_changed = true;
        }
    }
}

/**
 * Keyboard Mode
 **/
void key_mode()
{
    if (tud_hid_ready())
    {
        /*------------- Keyboard -------------*/
        if (sw_changed)
        {
            bool is_pressed = false;
            int keycode_idx = 0;
            uint8_t keycode[6] = {0}; //looks like we are limited to 6kro?
            for (int i = 0; i < SW_GPIO_SIZE; i++)
            {
                if (sw_val[i])
                {
                    // use to avoid send multiple consecutive zero report for keyboard
                    keycode[keycode_idx] = SW_KEYCODE[i];
                    keycode_idx = ++keycode_idx % SW_GPIO_SIZE;
                    is_pressed = true;

                    prev_sw_val[i] = sw_val[i];
                }
            }
            if (is_pressed)
            {
                // Send key report
                tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
            }
            else
            {
                // Send empty key report if previously has key pressed
                tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            }
            sw_changed = false;
        }
    }
}

/**
 * Main Loop Function
 **/
int main(void)
{
    board_init();
    tusb_init();
    init();

    while (1)
    {
        tud_task(); // tinyusb device task
        update_inputs();
        key_mode();
    }

    return 0;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    // TODO set LED based on CAPLOCK, NUMLOCK etc...
    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {
        if (buffer[0] == 0x1)
            gpio_put(25, true);
        else
            gpio_put(25, false);
    }
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}
