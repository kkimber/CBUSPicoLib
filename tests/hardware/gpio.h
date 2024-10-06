// FAKE STUB HEADER

#pragma once

#include <cstdint>

#define bi_decl(x)

typedef uint8_t gpio_function_t;

static constexpr const gpio_function_t GPIO_FUNC_I2C {0};

void gpio_set_function (uint gpio, gpio_function_t fn) {};