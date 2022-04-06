#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "pico/stdlib.h"

struct MouseEvent {
    uint8_t keys;
    uint8_t x;
    uint8_t y;
};

bool sendMouseEvent(queue_t *queue, uint8_t keys, uint8_t x, uint8_t y);

#endif