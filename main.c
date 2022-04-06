/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define PERIOD_MS 10

void init() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

int main() {

    init();
    tusb_init();

    int32_t last = to_ms_since_boot(get_absolute_time());
    while (true) {
        if (to_ms_since_boot(get_absolute_time()) - last >= PERIOD_MS) {
            if (tud_hid_ready())
                tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, 5, 0, 0, 0);
            gpio_put(LED_PIN, !gpio_get(LED_PIN));
            last = to_ms_since_boot(get_absolute_time());
        }
        tud_task();
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) instance;
}