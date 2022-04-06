#include "utils.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

bool sendMouseEvent(queue_t *queue, const void* data) {
    return queue_try_add(queue, data);
}
