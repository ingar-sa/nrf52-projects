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


#define LOG_MODULE_NAME app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

static void flash_led1(void);

static void
button1_pressed(uint32_t button_state, uint32_t has_changed) {
	// printk("Button state: %i | State has changed: %i\n", button_state, has_changed);
	/*
	 * Button 1 has has_changed = 1;
	 * Button 2 has has_changed = 2;
	 * Button 3 has has_changed = 4;
	 * Button 4 has has_changed = 8;
	 */
	// uint32_t button_pressed = has_changed; 

	// switch (button_pressed)
	// {
	// case 4:
	// 	button_pressed = 3;
	// 	break;
	// case 8:
	// 	button_pressed = 4;
	// 	break;
	// default:
	// 	// LOG_ERR("Invalid button (err: %d)", button1_pressed);
	// 	break;
	// }

	// // button_state is zero before the button is pressed
	// if (!button_state) {
	// 	printk("Button %i pressed", button_pressed);
	// }

	uint32_t button_pressed = 0;
	if (has_changed & button_state) {
		switch (has_changed)
		{
		case DK_BTN3_MSK:
			button_pressed = 3;
			break;
		case DK_BTN4_MSK:
			button_pressed = 4;
			break;
		default:
			// For button 1 has_changed = 1, and for button 2 it's 2
			button_pressed = has_changed;
			break;
		}

		LOG_INF("Button %d pressed.", button_pressed);
	}
}

static void
configure_dk_buttons_leds(void) {
	int err;
	err = dk_leds_init();
	if (err) {
		LOG_ERR("Cannot init leds (err: %d)", err);
	}

	err = dk_buttons_init(button1_pressed);
	if (err) {
		LOG_ERR("Could not initialize buttons (err: %d)", err);
	}
}

static void
flash_led1(void) {

	// int err;
	bool on_or_off = false;
	for (;;) {
		// err = dk_set_led(RUN_STATUS_LED, on_or_off);
		// if (err) {
		// 	LOG_ERR("Could not toggleLED 1 (err: %d", err);
		// }
		// dk_set_led(RUN_STATUS_LED, (on_or_off++)%2);
		 dk_set_led(RUN_STATUS_LED, on_or_off);
		 k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
		// LOG_INF("on_or_off is: %d", on_or_off);
		
		on_or_off = (on_or_off) ? false : true;
	}
}

void main(void)
{	
	printk("Hello World! %s\n", CONFIG_BOARD);
	// printk("New version flashed\n");
	configure_dk_buttons_leds();
	int err = mpu_sensor_init();
	if (err) {
		LOG_ERR("Failed to initialize MPU sensor! (err: %d)", err);
	}

	flash_led1();
}

