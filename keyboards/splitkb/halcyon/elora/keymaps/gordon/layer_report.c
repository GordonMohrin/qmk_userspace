// Copyright 2026 Gordon Mohrin
// SPDX-License-Identifier: GPL-2.0-or-later

// Reports the active layers and modifiers over raw HID for Halcyon Type
// (github.com/GordonMohrin/halcyon-type), also while Vial is locked.
// Only the layer and modifier state leaves the keyboard, never which keys are
// pressed, so this does not weaken Vial's protection against keyloggers.
//
// Request:  0x02 (id_get_keyboard_value), 0x48
// Response: 0x02, 0x48, version, layer_state (4 bytes BE), default_layer_state (4 bytes BE),
//           mods, oneshot mods, flags (bit 0 Caps Word, bit 1 Caps Lock)

#include QMK_KEYBOARD_H
#include "via.h"

#define ID_HALCYON_STATE 0x48
#define HALCYON_STATE_VERSION 1

static void put_be32(uint8_t *p, uint32_t v) {
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v;
}

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    if (data[0] == id_get_keyboard_value && data[1] == ID_HALCYON_STATE) {
        data[2] = HALCYON_STATE_VERSION;
        put_be32(&data[3], (uint32_t)layer_state);
        put_be32(&data[7], (uint32_t)default_layer_state);
        data[11]      = get_mods();
        data[12]      = get_oneshot_mods();
        uint8_t flags = 0;
#ifdef CAPS_WORD_ENABLE
        if (is_caps_word_on()) flags |= 1;
#endif
        if (host_keyboard_led_state().caps_lock) flags |= 2;
        data[13] = flags;
        return;
    }
    data[0] = id_unhandled;
}
