#include "user_presence.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define USER_BUTTON_GPIO 15   // GPIO pin for button

void user_presence_init(void) {
    gpio_init(USER_BUTTON_GPIO);
    gpio_set_dir(USER_BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(USER_BUTTON_GPIO);   // active LOW
}

bool user_presence_wait(uint32_t timeout_ms) {
    absolute_time_t start = get_absolute_time();

    while (absolute_time_diff_us(start, get_absolute_time()) < timeout_ms * 1000) {
        if (gpio_get(USER_BUTTON_GPIO) == 0) {
            sleep_ms(40); // debounce
            return true;
        }
        tight_loop_contents();
    }
    return false;
}
