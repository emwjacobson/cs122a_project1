#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "pico/stdlib.h"

struct MouseEvent {
    uint8_t keys;
    uint8_t x;
    uint8_t y;
};

struct TaskStruct {
    int16_t period_ms;
    int32_t last_ms;
    int (*tick_fn)(int);
    int cur_state;
};

bool sendMouseEvent(queue_t *queue, const void* data);

#endif