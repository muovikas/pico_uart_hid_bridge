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
#include "hardware/gpio.h"
#include "hardware/uart.h"

// UART configuration
#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// HID keycodes for keypad and special keys
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
#define HID_KEYPAD_SLASH    0x54
#define HID_KEYPAD_MINUS    0x56
#define HID_KEYPAD_PLUS     0x57
#define HID_KEY_BACKSPACE   0x2A

// Sequence engine states
typedef enum {
    SEQ_IDLE,
    SEQ_PRESS,
    SEQ_RELEASE
} seq_state_t;

// Sequence structure for HID key(s)
typedef struct {
    const uint8_t* seq;
    size_t len;
} keyseq_t;

// Single-key sequence buffer
static uint8_t single_seq[2] = {0, 0};
static keyseq_t sequences[1] = {
    { single_seq, 1 }
};

static int current_sequence = 0;
static size_t key_idx = 0;
static seq_state_t seq_state = SEQ_IDLE;
static absolute_time_t seq_timer = {0};

// Handles sending HID key press/release using the sequence engine
void send_sequence_task(void) {
    if (!tud_hid_ready()) return;

    absolute_time_t now = get_absolute_time();

    switch (seq_state) {
        case SEQ_IDLE:
            // Do nothing, wait for UART to trigger a sequence
            break;
        case SEQ_PRESS:
            // Send HID key press
            if (key_idx < sequences[current_sequence].len) {
                uint8_t key = sequences[current_sequence].seq[key_idx];
                uint8_t keycodes[6] = {0};
                keycodes[0] = key;
                tud_hid_keyboard_report(0, 0, keycodes);
                seq_state = SEQ_RELEASE;
                seq_timer = now;
            } else {
                seq_state = SEQ_IDLE;
            }
            break;
        case SEQ_RELEASE:
            // Release HID key after 100ms
            if (absolute_time_diff_us(seq_timer, now) > 100000) { // 100ms
                tud_hid_keyboard_report(0, 0, NULL);
                key_idx++;
                seq_state = SEQ_PRESS;
                seq_timer = now;
            }
            break;
    }
}

// Starts a new single-key HID sequence (called from UART handler)
void start_single_key_sequence(uint8_t keycode) {
    single_seq[0] = keycode;
    single_seq[1] = 0; // End marker (not used)
    sequences[0].seq = single_seq;
    sequences[0].len = 1;
    current_sequence = 0;
    key_idx = 0;
    seq_state = SEQ_PRESS;
    seq_timer = get_absolute_time();
}

int main(void)
{
    board_init();
    tusb_init();

    // Initialize UART at 9600 8N1
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    seq_state = SEQ_IDLE;

    while (1)
    {
        tud_task();

        // UART receive and HID trigger
        if (uart_is_readable(UART_ID)) {
            uint8_t ch = uart_getc(UART_ID);
            uart_putc(UART_ID, ch); // Echo received character

            // Map UART character to HID keycode
            uint8_t keycode = 0;
            switch (ch) {
                case '0': keycode = HID_KEYPAD_0; break;
                case '1': keycode = HID_KEYPAD_1; break;
                case '2': keycode = HID_KEYPAD_2; break;
                case '3': keycode = HID_KEYPAD_3; break;
                case '4': keycode = HID_KEYPAD_4; break;
                case '5': keycode = HID_KEYPAD_5; break;
                case '6': keycode = HID_KEYPAD_6; break;
                case '7': keycode = HID_KEYPAD_7; break;
                case '8': keycode = HID_KEYPAD_8; break;
                case '9': keycode = HID_KEYPAD_9; break;
                case '*': keycode = HID_KEYPAD_ASTERISK; break;
                case '/': keycode = HID_KEYPAD_SLASH; break;
                case '-': keycode = HID_KEYPAD_MINUS; break;
                case '+': keycode = HID_KEYPAD_PLUS; break;
                case '\b': keycode = HID_KEY_BACKSPACE; break;
                case '\r':
                case '\n': keycode = HID_KEYPAD_ENTER; break;
                default: break;
            }

            // Print debug message to UART
            char msg[64];
            snprintf(msg, sizeof(msg),
                "Received character 0x%02X ('%c') from UART, send HID %s\r\n",
                ch, (ch >= 32 && ch <= 126) ? ch : '.', keycode ? "YES" : "NO");
            for (char *p = msg; *p; ++p) uart_putc(UART_ID, *p);

            // If mapped, start HID sequence
            if (keycode) {
                start_single_key_sequence(keycode);
            }
        }

        // Run the HID sequence engine
        send_sequence_task();
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