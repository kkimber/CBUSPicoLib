// FAKE STUB HEADER

#pragma once

#include <cstdint>

uint64_t make_timeout_time_ms(uint64_t)
{
   return 0;
}

void busy_wait_ms (uint32_t delay_ms)
{
}

static uint32_t save_and_disable_interrupts (void)
{
   return 0UL;
}

static void restore_interrupts (uint32_t status)
{
}


