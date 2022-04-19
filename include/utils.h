#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"

struct KeyboardEvent {
    uint8_t modifiers;
    uint8_t keys[6];
};

struct MouseEvent {
    uint8_t keys;
    uint8_t x;
    uint8_t y;
};

struct HIDEvent {
    enum { EVENT_KEYBOARD, EVENT_MOUSE } type;
    struct MouseEvent mouse_data;
    struct KeyboardEvent keyboard_data;
};

bool sendMouseEvent(queue_t *queue, uint8_t keys, uint8_t x, uint8_t y);
bool sendKeyboardEvent(queue_t *queue, uint8_t modifiers, uint8_t keys[6]);
void logMessage(char* str);
void logLine(char* str);
uint16_t readADC(uint8_t num);
long map(long x, long in_min, long in_max, long out_min, long out_max);

#endif