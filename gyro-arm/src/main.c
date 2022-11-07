/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <sys/printk.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>
#include <mpu_sensor.h>
#include <motor_control.h>


#define LOG_MODULE_NAME app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

static void
button_pressed(uint32_t button_state, uint32_t has_changed) {
	uint8_t button_pressed;
	if (has_changed & button_state) {
		switch (has_changed)
		{
		case DK_BTN1_MSK:
			button_pressed = 1;
			break;
		case DK_BTN2_MSK:
			button_pressed = 2;
			break;
		case DK_BTN3_MSK:
			button_pressed = 3;
			break;
		case DK_BTN4_MSK:
			button_pressed = 4;
			break;
		default:
			LOG_ERR("Invalid button (err: %d)", button_state);
			break;
		}

		set_motor_angle(button_pressed);
		LOG_INF("Button %d pressed.");
	}
}

static void
configure_dk_buttons_leds(void) {
	int err;
	err = dk_leds_init();
	if (err) {
		LOG_ERR("Cannot init leds (err: %d)", err);
	}

	err = dk_buttons_init(button_pressed);
	if (err) {
		LOG_ERR("Could not initialize buttons (err: %d)", err);
	}
}

int main(void)
{	
	printk("Hello World! %s\n", CONFIG_BOARD);
	printk("New version flashed\n");

	struct accel_values accel_values;
	struct gyro_values gyro_values;

	configure_dk_buttons_leds();

	int err = 0;
	err = mpu_sensor_init();
	if (err) {
		LOG_ERR("Failed to initialize MPU sensor! (err: %d)", err);
	}

	err = motor_init();
	if (err) {
		LOG_ERR("Failed to initialize motor. Error: %d", err);
	}

	int on_or_off = 1;
	for (;;) {
		// dk_set_led(RUN_STATUS_LED, (on_or_off++)%2);
		if (read_accel_values(&accel_values) == 0) {
			LOG_INF("# %d, Accel: X %06d, Y: %06d, Z: %06d", on_or_off, accel_values.x, accel_values.y, accel_values.z);
		}
		if (read_gyro_values(&gyro_values) == 0) {
			LOG_INF("# %d, Gyro: X %06d, Y: %06d, Z: %06d", on_or_off, gyro_values.x, gyro_values.y, gyro_values.z);
		}
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
	
	return 0;
}

