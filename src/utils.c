#include "utils.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

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
