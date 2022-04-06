/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define POLL_PERIOD_MS 100

struct MouseEvent {
    uint8_t x;
    uint8_t y;
};

queue_t queue;

void init() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    queue_init(&queue, sizeof(struct MouseEvent), 10);
}

int main() {
    init();
    tusb_init();

    uint32_t last_ms = to_ms_since_boot(get_absolute_time());
    while(1) {
        uint32_t cur_ms = to_ms_since_boot(get_absolute_time());
        if (cur_ms - last_ms > POLL_PERIOD_MS) {
            struct MouseEvent data = {
                .x = 10,
                .y = 0
            };
            bool res = queue_try_add(&queue, &data);
            last_ms = cur_ms;
        }

        // Process mouse movement items in the queue
        // If ready to send HID data and queue has items to process
        if (tud_hid_ready() && !queue_is_empty(&queue)) {
            struct MouseEvent data;
            queue_try_remove(&queue, &data);

            tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, data.x, data.y, 0, 0);
        }
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