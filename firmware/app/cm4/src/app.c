#include "cm4/app.h"

#include <stdint.h>

static volatile uint32_t s_step_counter = 0;

void cm4_app_init(void) {
    s_step_counter = 0;
}

void cm4_app_step(void) {
    s_step_counter++;
}
