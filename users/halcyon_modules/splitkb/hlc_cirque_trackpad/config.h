// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

// Any QMK options should go here

#pragma once

#define HLC_CIRQUE_TRACKPAD

#define CIRQUE_PINNACLE_DIAMETER_MM 35
#undef POINTING_DEVICE_CS_PIN
#define POINTING_DEVICE_CS_PIN 13
#define POINTING_DEVICE_ROTATION_180
#define CIRQUE_PINNACLE_CURVED_OVERLAY

#define CIRQUE_PINNACLE_POSITION_MODE CIRQUE_PINNACLE_ABSOLUTE_MODE

// Each feature can be switched off from a keymap's config.h
#ifndef HLC_CIRQUE_DISABLE_GLIDE
#    define POINTING_DEVICE_GESTURES_CURSOR_GLIDE_ENABLE
#endif
#ifndef HLC_CIRQUE_DISABLE_TAP
#    define CIRQUE_PINNACLE_TAP_ENABLE
#endif
#ifndef HLC_CIRQUE_DISABLE_SCROLL
#    define POINTING_DEVICE_GESTURES_SCROLL_ENABLE
#endif
