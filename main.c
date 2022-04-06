#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "utils.h"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define NUM_SMS 2

queue_t queue;

void init() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    queue_init(&queue, sizeof(struct MouseEvent), 10);
}

enum PG_STATES { PG_START, PG_WAIT, PG_SEND };
int Playground_Tick(int cur_state) {
    static int move = 10;
    switch (cur_state) {
        case PG_START:
            cur_state = PG_WAIT;
            break;
        case PG_WAIT:
            cur_state = PG_SEND;
            break;
        case PG_SEND:
            cur_state = PG_WAIT;
            break;
    }

    switch (cur_state) {
        case PG_START:
            break;
        case PG_WAIT:
            break;
        case PG_SEND:
            sendMouseEvent(&queue, 0x00, move, 0);
            move *= -1;
            break;
    }
}

enum LED_STATES { LED_START, LED_TOGGLE };
int LED_Tick(int cur_state) {
    switch (cur_state) {
        case LED_START:
            gpio_put(LED_PIN, 0);
            cur_state = LED_TOGGLE;
            break;
        case LED_TOGGLE:
            cur_state = LED_TOGGLE;
            break;
    }

    switch (cur_state) {
        case LED_START:
            break;
        case LED_TOGGLE:
            gpio_put(LED_PIN, ~gpio_get(LED_PIN));
            break;
    }
}

struct TaskStruct {
    int16_t period_ms;
    int32_t last_ms;
    int (*tick_fn)(int);
    int cur_state;
};

int main() {
    init();
    tusb_init();

    uint32_t last_ms = to_ms_since_boot(get_absolute_time());

    struct TaskStruct tasks[NUM_SMS];
    // *** DONT FORGET TO MODIFY NUM_SMS ***

    // Playground
    tasks[0].period_ms = 100;
    tasks[0].last_ms = last_ms;
    tasks[0].tick_fn = Playground_Tick;
    tasks[0].cur_state = PG_START;

    // LED Blinking
    tasks[1].period_ms = 1000;
    tasks[1].last_ms = last_ms;
    tasks[1].tick_fn = LED_Tick;
    tasks[1].cur_state = LED_START;

    while(1) {
        uint32_t cur_ms = to_ms_since_boot(get_absolute_time());
        
        for (int i=0; i<NUM_SMS;i++) {
            if (cur_ms - last_ms > tasks[i].period_ms)
                tasks[i].cur_state = tasks[i].tick_fn(tasks[i].cur_state);
        }

        // Process mouse movement items in the queue
        // If ready to send HID data and queue has items to process
        if (tud_hid_ready() && !queue_is_empty(&queue)) {
            struct MouseEvent data;
            queue_try_remove(&queue, &data);

            tud_hid_mouse_report(REPORT_ID_MOUSE, data.keys, data.x, data.y, 0, 0);
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