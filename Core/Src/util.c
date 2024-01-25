/*
 * util.c
 *
 *  Created on: Jan 5, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This file contains utility functions for the game.
 */

#include "util.h"

void delay_us(TIM_HandleTypeDef *htim, uint16_t us) {
	__HAL_TIM_SET_COUNTER(htim, 0);
	while (__HAL_TIM_GET_COUNTER(htim) < us) {
		asm("nop");
	}
}
