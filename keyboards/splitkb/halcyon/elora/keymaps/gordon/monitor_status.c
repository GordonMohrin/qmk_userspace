// Copyright 2026 Gordon Mohrin
// SPDX-License-Identifier: GPL-2.0-or-later

// Shows on the keyboard which display the PC is using, as reported by the
// "monitorschalter" tray tool over raw HID.
//
// Host -> keyboard: 0x03 (id_set_keyboard_value), 0x49, status
//   status 1 = main monitor active, 2 = remote mode (dummy display only)
// The tool repeats the message every few seconds. Without a message for
// MONITOR_TIMEOUT ms the status falls back to "no contact".
//
// LEDs (right half, top row): 71 = key "9" (main monitor), 72 = key "0" (remote)
//   main monitor -> "9" green, "0" dark
//   remote mode  -> "0" blue,  "9" dark
//   no contact   -> both blink yellow
//
// The master half receives the raw HID message and forwards the status to
// the other half, so it works no matter which half is plugged in.

#include QMK_KEYBOARD_H
#include "via.h"
#include "transactions.h"

#define ID_MONITOR_STATUS 0x49
#define MONITOR_TIMEOUT 15000
#define LED_KEY_MAIN 71
#define LED_KEY_REMOTE 72

enum { MON_NONE = 0, MON_MAIN = 1, MON_REMOTE = 2 };

static uint8_t  host_status   = MON_NONE; // last status from the host (master)
static uint32_t host_seen     = 0;        // timer of the last message (master)
static uint8_t  shown_status  = MON_NONE; // status the LEDs show (both halves)
static uint8_t  synced_status = 0xFF;     // last status sent to the other half
static uint32_t synced_at     = 0;

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    if (data[0] == id_set_keyboard_value && data[1] == ID_MONITOR_STATUS) {
        uint8_t s = data[2];
        if (s == MON_MAIN || s == MON_REMOTE) {
            host_status = s;
            host_seen   = timer_read32();
        }
        data[2] = 1; // protocol version, lets the host check the firmware
        return;
    }
    data[0] = id_unhandled;
}

static void monitor_sync_handler(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    shown_status = *(const uint8_t *)in_data;
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(USER_MONITOR_SYNC, monitor_sync_handler);
}

void housekeeping_task_user(void) {
    if (!is_keyboard_master()) {
        return;
    }
    uint8_t s = host_status;
    if (s != MON_NONE && timer_elapsed32(host_seen) > MONITOR_TIMEOUT) {
        s = host_status = MON_NONE;
    }
    shown_status = s;
    // Send on change, and once a second so a restarted half catches up.
    if (s != synced_status || timer_elapsed32(synced_at) > 1000) {
        if (transaction_rpc_send(USER_MONITOR_SYNC, sizeof(s), &s)) {
            synced_status = s;
        }
        synced_at = timer_read32();
    }
}

static void set_led(uint8_t index, uint8_t led_min, uint8_t led_max, uint8_t h, uint8_t s, uint8_t on) {
    if (index < led_min || index >= led_max) {
        return;
    }
    if (!on) {
        rgb_matrix_set_color(index, 0, 0, 0);
        return;
    }
    hsv_t hsv = {h, s, rgb_matrix_get_val()};
    rgb_t rgb = hsv_to_rgb(hsv);
    rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    switch (shown_status) {
        case MON_MAIN:
            set_led(LED_KEY_MAIN, led_min, led_max, 85, 255, 1); // green
            set_led(LED_KEY_REMOTE, led_min, led_max, 0, 0, 0);
            break;
        case MON_REMOTE:
            set_led(LED_KEY_MAIN, led_min, led_max, 0, 0, 0);
            set_led(LED_KEY_REMOTE, led_min, led_max, 170, 255, 1); // blue
            break;
        default: {
            uint8_t on = (timer_read32() / 500) % 2;
            set_led(LED_KEY_MAIN, led_min, led_max, 43, 255, on); // yellow
            set_led(LED_KEY_REMOTE, led_min, led_max, 43, 255, on);
            break;
        }
    }
    return false;
}
