#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"

struct MouseEvent {
    uint8_t keys;
    uint8_t x;
    uint8_t y;
};

bool sendMouseEvent(queue_t *queue, uint8_t keys, uint8_t x, uint8_t y);
void logMessage(char* str);
void logLine(char* str);
uint16_t readADC(uint8_t num);
long map(long x, long in_min, long in_max, long out_min, long out_max);

#endif