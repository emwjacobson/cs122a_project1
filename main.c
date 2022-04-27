#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "utils.h"
#include "hardware/adc.h"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define NUM_SMS 4
#define DEADZONE 50
#define ADC0 26
#define ADC1 27
#define JS_BUTTON 15

// ***** Global SM Variables *****
int16_t js_x;
int16_t js_y;
bool js_button;
enum MODES {
    MODE_PAN = 0,
    MODE_ROTATE,
    MODE_MAX
} mode;
// *******************************

queue_t queue;

void init() {
    stdio_init_all();

    // Initialize onboard LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    // Initialize ADCs and button port
    adc_init();
    adc_gpio_init(ADC0);
    adc_gpio_init(ADC1);
    gpio_init(JS_BUTTON);
    gpio_set_dir(JS_BUTTON, GPIO_IN);
    gpio_pull_up(JS_BUTTON);

    // Initialize queue for mouse events
    queue_init(&queue, sizeof(struct HIDEvent), 10);

    mode = MODE_PAN;
}

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
            // gpio_put(LED_PIN, !gpio_get(LED_PIN));
            gpio_put(LED_PIN, mode == MODE_PAN);
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

            // Button is pulled up, so make sure to invert the input to get
            // the "logical" usage of button. 1 = pressed, 0 = unpressed
            js_button = !gpio_get(JS_BUTTON);

            break;
    }

    return cur_state;
}

// *****
// Mode SM
// Used joystick button to change mode
// *****
enum MD_STATES { MD_START, MD_WAIT, MD_HOLD, MD_TOGGLE };
int Mode_Tick(int cur_state) {
    switch (cur_state) {
        case MD_START:
            cur_state = MD_WAIT;
            break;
        case MD_WAIT:
            cur_state = (js_button == 1 ? MD_HOLD : MD_WAIT);
            break;
        case MD_HOLD:
            cur_state = (js_button? MD_HOLD : MD_TOGGLE);
            break;
        case MD_TOGGLE:
            cur_state = MD_WAIT;
            break;
    }

    switch (cur_state) {
        case MD_WAIT:
            break;
        case MD_HOLD:
            break;
        case MD_TOGGLE:
            mode = (mode + 1) % (MODE_MAX);
            break;
    }

    return cur_state;
}

// *****
// Move SM
// This adds the mouse movement event to the queue.
// A mouse movement should consist of a few different stages:
//      Movement Preamble: The initial press keystrokes (eg send a SHIFT key so mouse movements pan instead of rotate)
//      Movement Action: The actual mouse events
//      Movement Epilogue: The release of the keystrokes sent in the preamble
// *****
enum MV_STATES { MV_START, MV_WAIT, MV_PREAMBLE, MV_ACTION, MV_EPILOGUE };
int Move_Tick(int cur_state) {
    static uint8_t active_keys[6] = { 0, 0, 0, 0, 0 };
    switch (cur_state) {
        case MV_START:
            cur_state = MV_WAIT;
            break;
        case MV_WAIT:
            // Wait for a movement that != 0
            if (js_x != 0 || js_y != 0) {
                cur_state = MV_PREAMBLE;
            } else {
                cur_state = MV_WAIT;
            }
            break;
        case MV_PREAMBLE:
            cur_state = MV_ACTION;
            break;
        case MV_ACTION:
            if (js_x != 0 || js_y != 0) {
                cur_state = MV_ACTION;
            } else {
                cur_state = MV_EPILOGUE;
            }
            break;
        case MV_EPILOGUE:
            cur_state = MV_WAIT;
            break;
    }

    switch (cur_state) {
        case MV_START:
            break;
        case MV_WAIT:
            break;
        case MV_PREAMBLE:
            if (mode == MODE_PAN) {
                // Press left shift
                sendKeyboardEvent(&queue, KEYBOARD_MODIFIER_LEFTCTRL, active_keys);
            }
            break;
        case MV_ACTION:
            sendMouseEvent(&queue, MOUSE_BUTTON_MIDDLE, js_x, js_y);
            break;
        case MV_EPILOGUE:
            if (mode == MODE_PAN) {
                // Release left shift
                sendKeyboardEvent(&queue, 0x00, active_keys);
            }
            sendMouseEvent(&queue, 0x00, 0x00, 0x00);
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

    // Poll Joystick Button
    tasks[2].period_ms = 100;
    tasks[2].last_ms = 0;
    tasks[2].tick_fn = &Mode_Tick;
    tasks[2].cur_state = MD_START;

    // Move Mouse
    tasks[3].period_ms = 20;
    tasks[3].last_ms = 0;
    tasks[3].tick_fn = &Move_Tick;
    tasks[3].cur_state = MV_START;

    int32_t cur_ms;
    int32_t last_push = 0;
    char message[64];

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
            struct HIDEvent data;
            bool item = queue_try_remove(&queue, &data);

            struct KeyboardEvent k_data = data.keyboard_data;
            struct MouseEvent m_data = data.mouse_data;

            if (item) {
                cur_ms = to_ms_since_boot(get_absolute_time());
                switch(data.type) {
                    case EVENT_KEYBOARD:
                        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, k_data.modifiers, k_data.keys);
                        break;
                    case EVENT_MOUSE:
                        tud_hid_mouse_report(REPORT_ID_MOUSE, m_data.keys, m_data.x, m_data.y, 0, 0);
                        snprintf(message, 64, "Mouse: %i  X: %i  Y: %i\n", m_data.keys, m_data.x, m_data.y);
                        logLine(message);
                        break;
                }
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