/*
 * rng.h
 *
 *  Created on: Jan 4, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This is a simple random number generator for use with the STM32 timers.
 */

#ifndef INC_RNG_H_
#define INC_RNG_H_

#include "main.h"
#include "stdint.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;

void rng_seed(uint32_t seed);
uint32_t rng_get(TIM_HandleTypeDef *htim_a, TIM_HandleTypeDef *htim_b, uint32_t max_value);

#endif /* INC_RNG_H_ */
