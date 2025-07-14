/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"
//#include "keyboard.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// This is for debugging feature
#define HID_KEY_A           0x04
#define HID_KEYPAD_0        0x62
#define HID_KEYPAD_1        0x59
#define HID_KEYPAD_2        0x5A
#define HID_KEYPAD_3        0x5B
#define HID_KEYPAD_4        0x5C
#define HID_KEYPAD_5        0x5D
#define HID_KEYPAD_6        0x5E
#define HID_KEYPAD_7        0x5F
#define HID_KEYPAD_8        0x60
#define HID_KEYPAD_9        0x61
#define HID_KEYPAD_ASTERISK 0x55
#define HID_KEYPAD_ENTER    0x58


//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+


void hid_task(void);

static void send_hid_report(bool keys_pressed, const uint8_t keycodes[6])
{
    if (!tud_hid_ready())
        return;

    if (keys_pressed)
    {
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycodes);
    }
    else
    {
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
    }
}


void send_test() {
    uint8_t keycode = HID_KEY_A;
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, &keycode);
    sleep_ms(10);
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
    sleep_ms(5);
}

/*------------- MAIN -------------*/
int main(void)
{
    board_init();
    tusb_init();

    static absolute_time_t last_poll = {0};

    while (1)
    {
        tud_task();

        // Poll every 10ms
        if (absolute_time_diff_us(last_poll, get_absolute_time()) > 2000000) {
            last_poll = get_absolute_time();
            send_test(); // Send a test report every 2000ms
        }
    }
    return 0;
}

//--------------------------------------------------------------------+
// USB HID callbacks
//--------------------------------------------------------------------+


void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
    (void)instance;
    (void)len;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    (void)instance;
    (void)report_type;
    (void)report_id;
    (void)buffer;
    (void)bufsize;
}