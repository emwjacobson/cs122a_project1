#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "utils.h"
#include "hardware/adc.h"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define NUM_SMS 3
#define DEADZONE 50
#define ADC0 26
#define ADC1 27

queue_t queue;

void init() {
    stdio_init_all();

    // Initialize onboard LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    // Initialize ADCs
    adc_init();
    adc_gpio_init(ADC0);
    adc_gpio_init(ADC1);

    // Initialize queue for mouse events
    queue_init(&queue, sizeof(struct MouseEvent), 10);
}

// ***** Global SM Variables *****
int16_t js_x;
int16_t js_y;
// *******************************

// *****
// LED SM
// Makes the onboard LED flash
// *****
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
            gpio_put(LED_PIN, !gpio_get(LED_PIN));
            break;
    }

    return cur_state;
}

// *****
// Joystick SM
// SM that polls the the joystick
// Joystick value goes from 0 to 4096, center is 2048
// *****
enum JS_STATES { JS_START, JS_POLL };
int JS_Tick(int cur_state) {
    static int16_t raw_x, raw_y;
    switch(cur_state) {
        case JS_START:
            cur_state = JS_POLL;
            break;
        case JS_POLL:
            cur_state = JS_POLL;
            break;
    }

    switch(cur_state) {
        case JS_POLL:
            raw_x = readADC(1) - 2048; // X is ADC1
            raw_y = readADC(0) - 2048; // Y is ADC0

            if (raw_x <= DEADZONE && raw_x >= -DEADZONE) {
                raw_x = 0;
            } else {
                raw_x = raw_x;
            }

            if (raw_y <= DEADZONE && raw_y >= -DEADZONE) {
                raw_y = 0;
            } else {
                raw_y = raw_y;
            }

            js_x = map(raw_x, -2048, 2048, -20, 20);
            js_y = map(raw_y, -2048, 2048, -20, 20);

            break;
    }

    return cur_state;
}

// *****
// Playground SM
// Just a place to test new things
// *****
enum PG_STATES { PG_START, PG_MOVE };
int Playground_Tick(int cur_state) {
    switch (cur_state) {
        case PG_START:
            cur_state = PG_MOVE;
            break;
        case PG_MOVE:
            cur_state = PG_MOVE;
            break;
    }

    switch (cur_state) {
        case PG_START:
            cur_state = PG_MOVE;
            break;
        case PG_MOVE:
            cur_state = PG_MOVE;
            sendMouseEvent(&queue, 0x00, js_x, js_y);
            break;
    }

    return cur_state;
}

struct TaskStruct {
    int period_ms;
    int last_ms;
    int (*tick_fn)(int);
    int cur_state;
};

int main() {
    sleep_ms(1000);

    init();
    tusb_init();

    struct TaskStruct tasks[NUM_SMS];
    // *** DONT FORGET TO MODIFY NUM_SMS ***

    // LED Blinking
    tasks[0].period_ms = 100;
    tasks[0].last_ms = 0;
    tasks[0].tick_fn = &LED_Tick;
    tasks[0].cur_state = LED_START;

    // Joystick Polling
    tasks[1].period_ms = 10;
    tasks[1].last_ms = 0;
    tasks[1].tick_fn = &JS_Tick;
    tasks[1].cur_state = JS_START;

    // Playground
    tasks[2].period_ms = 20;
    tasks[2].last_ms = 0;
    tasks[2].tick_fn = &Playground_Tick;
    tasks[2].cur_state = PG_START;

    int32_t cur_ms;

    int32_t last_push = 0;
    char time[32];

    while(1) {
        cur_ms = to_ms_since_boot(get_absolute_time());
        for (int i=0; i<NUM_SMS;i++) {
            if (cur_ms - tasks[i].last_ms >= tasks[i].period_ms) {
                tasks[i].cur_state = tasks[i].tick_fn(tasks[i].cur_state);
                tasks[i].last_ms = cur_ms;
            }
            
        }

        // Process mouse movement items in the queue
        // If ready to send HID data and queue has items to process
        if (tud_hid_ready() && !queue_is_empty(&queue)) {
            struct MouseEvent data;
            bool item = queue_try_remove(&queue, &data);

            if (item) {
                cur_ms = to_ms_since_boot(get_absolute_time());
                tud_hid_mouse_report(REPORT_ID_MOUSE, data.keys, data.x, data.y, 0, 0);
                snprintf(time, 32, "%i", (cur_ms - last_push));
                last_push = cur_ms;
            }
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