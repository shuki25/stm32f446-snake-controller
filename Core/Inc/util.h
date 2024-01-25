/*
 * util.h
 *
 *  Created on: Jan 5, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This file contains utility functions for the project.
 */

#ifndef INC_UTIL_H_
#define INC_UTIL_H_

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include <stdint.h>

void delay_us(TIM_HandleTypeDef *htim ,uint16_t us);


#endif /* INC_UTIL_H_ */
