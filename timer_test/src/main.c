/*
 * Copyright (c) 2025 3mdeb Sp z o.o.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>

#define SLEEP_TIME_MS 1000

int main(void)
{
  uint32_t uptime_ms = 0;

	while (1) {
		uptime_ms = k_uptime_get_32();
		printf("Uptime: %u ms\n", uptime_ms);

		uint32_t freq = sys_clock_hw_cycles_per_sec();
		printf("System clock frequency: %u Hz\n", freq);

		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
