/*
 * snes_controller.c
 *
 *  Created on: Jan 4, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This file contains the functions to read the SNES controller.
 *
 *		The SNES controller uses a shift register to send button presses to the SNES console.
 *
 *		1. The SNES console sends a LATCH pulse to the controller to initiate shift register scan.
 *		2. The SNES console then sends 16 clock pulses to the controller.
 *		3. The controller sends the state of the 12 buttons on the first 12 clock pulses.
 *
 *		The order of shift register returns the state of each button:
 *
 *		1. B
 *		2. Y
 *		3. Select
 *		4. Start
 *		5. Up
 *		6. Down
 *		7. Left
 *		8. Right
 *		9. A
 *		10. X
 *		11. L
 *		12. R
 *
 */

#include "snes_controller.h"
#include "cmsis_os.h"
#include "util.h"

void snes_controller_init(snes_controller_t *snes_controller, TIM_HandleTypeDef *htim ,GPIO_TypeDef *latch_port, uint16_t latch_pin, GPIO_TypeDef *clock_port, uint16_t clock_pin, GPIO_TypeDef *data_port, uint16_t data_pin) {
	snes_controller->htim = htim;
	snes_controller->latch_port = latch_port;
	snes_controller->latch_pin = latch_pin;
	snes_controller->clock_port = clock_port;
	snes_controller->clock_pin = clock_pin;
	snes_controller->data_port = data_port;
	snes_controller->data_pin = data_pin;
	snes_controller->is_active = 0;
	snes_controller->current_button_state = 0xfff0;
	snes_controller->previous_button_state = 0xfff0;
}

void snes_controller_latch(snes_controller_t *snes_controller) {
	HAL_GPIO_WritePin(snes_controller->latch_port, snes_controller->latch_pin,
			GPIO_PIN_SET);
	delay_us(snes_controller->htim, 12);
	HAL_GPIO_WritePin(snes_controller->latch_port, snes_controller->latch_pin,
			GPIO_PIN_RESET);
}

void snes_controller_clock(snes_controller_t *snes_controller) {
	HAL_GPIO_WritePin(snes_controller->clock_port, snes_controller->clock_pin,
			GPIO_PIN_RESET);
	delay_us(snes_controller->htim, 6);
	HAL_GPIO_WritePin(snes_controller->clock_port, snes_controller->clock_pin,
			GPIO_PIN_SET);
}

void snes_controller_read(snes_controller_t *snes_controller) {
	snes_controller->previous_button_state = snes_controller->current_button_state;
	snes_controller->current_button_state = 0;

	snes_controller_latch(snes_controller);

	for (int i = 15; i >= 0 ; i--) {
		snes_controller_clock(snes_controller);
		snes_controller->current_button_state |= HAL_GPIO_ReadPin(
				snes_controller->data_port, snes_controller->data_pin) << i;
		delay_us(snes_controller->htim, 6);
	}
	snes_controller->current_button_state = (~snes_controller->current_button_state) & 0xFFF0;
}

void snes_controller_read2(snes_controller_t *snes_controller1, snes_controller_t *snes_controller2) {
    snes_controller1->previous_button_state = snes_controller1->current_button_state;
    snes_controller1->current_button_state = 0;

    snes_controller2->previous_button_state = snes_controller2->current_button_state;
    snes_controller2->current_button_state = 0;

    snes_controller_latch(snes_controller1); // Latch both controllers using the same latch pin

    for (int i = 15; i >= 0 ; i--) {
        delay_us(snes_controller1->htim, 6); // Delay both controllers using the same timer
        snes_controller1->current_button_state |= HAL_GPIO_ReadPin(
                snes_controller1->data_port, snes_controller1->data_pin) << i;
        snes_controller2->current_button_state |= HAL_GPIO_ReadPin(
                snes_controller2->data_port, snes_controller2->data_pin) << i;
        snes_controller_clock(snes_controller1); // Clock both controllers using the same clock pin
    }
    snes_controller1->current_button_state = (~snes_controller1->current_button_state) & 0xFFF0;
    snes_controller2->current_button_state = (~snes_controller2->current_button_state) & 0xFFF0;
	if (snes_controller1->current_button_state == 0xfff0) {
		snes_controller1->is_active = 0;
	} else {
		snes_controller1->is_active = 1;
	}
	if (snes_controller2->current_button_state == 0xfff0) {
		snes_controller2->is_active = 0;
	} else {
		snes_controller2->is_active = 1;
	}
}
