/*
 * rng.c
 *
 *  Created on: Jan 4, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This file contains the functions to generate random numbers.
 */

#include "rng.h"

volatile uint32_t random_seed = 0;
extern uint8_t is_random_seed_shared;

void rng_seed(uint32_t seed) {
    random_seed = seed;
}

uint32_t rng_get(TIM_HandleTypeDef *htim_a, TIM_HandleTypeDef *htim_b, uint32_t max_value) {
    if (is_random_seed_shared) {
        random_seed = (RNG_PRIME * random_seed + RNG_INCREMENT) % RNG_MODULUS;
        return random_seed % max_value;
    } else {
        uint32_t counter = (uint32_t)__HAL_TIM_GET_COUNTER(htim_a);
        for (uint32_t i = 0; i < 32; i++) {
            asm("nop");
        }
        counter ^= (uint32_t)__HAL_TIM_GET_COUNTER(htim_b);
        random_seed = random_seed * 1103515245 + RNG_INCREMENT;
        counter += random_seed;
        return counter % max_value;
    }
}
