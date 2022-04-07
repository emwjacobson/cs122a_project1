#include "utils.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "tusb.h"
#include "hardware/adc.h"

/**
 * @brief Sends a mouse event to the queue to be processed later in event loop.
 * https://wiki.osdev.org/USB_Human_Interface_Devices
 * 
 * @param queue The queue to add the Mouse event too
 * @param keys A bitfield of mouse keys.
 * @param x Amount to move mouse in x direction
 * @param y Amount to move mouse in y direction
 * @return `true` when item successfully added to queue, `false` otherwise
 */
bool sendMouseEvent(queue_t *queue, uint8_t keys, uint8_t x, uint8_t y) {
    struct MouseEvent data = {
        .keys = keys,
        .x = x,
        .y = y
    };
    return queue_try_add(queue, &data);
}

inline void logMessage(char* str) {
    tud_cdc_write_str(str);
    tud_cdc_write_flush();
}

inline void logLine(char* str) {
    tud_cdc_write_str(str);
    tud_cdc_write_str("\r\n");
    tud_cdc_write_flush();
}

uint16_t readADC(uint8_t num) {
    adc_select_input(num);
    return adc_read();
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
