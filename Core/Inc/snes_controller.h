/*
 * snes_controller.h
 *
 *  Created on: Jan 4, 2024
 *      Author: Joshua Butler, MD, MHI
 */

#ifndef INC_SNES_CONTROLLER_H_
#define INC_SNES_CONTROLLER_H_

#include <stdint.h>
#include "main.h"

// SNES Controller Button Masks
#define SNES_B_MASK 0x8000 // 0b1000000000000000
#define SNES_Y_MASK 0x4000 // 0b0100000000000000
#define SNES_SELECT_MASK 0x2000
#define SNES_START_MASK 0x1000
#define SNES_UP_MASK 0x0800
#define SNES_DOWN_MASK 0x0400
#define SNES_LEFT_MASK 0x0200
#define SNES_RIGHT_MASK 0x0100
#define SNES_A_MASK 0x0080
#define SNES_X_MASK 0x0040
#define SNES_L_MASK 0x0020
#define SNES_R_MASK 0x0010

typedef struct snes_controller_t {
	TIM_HandleTypeDef *htim;
	GPIO_TypeDef *latch_port;
	uint16_t latch_pin;
	GPIO_TypeDef *clock_port;
	uint16_t clock_pin;
	GPIO_TypeDef *data_port;
	uint8_t is_active;
	uint16_t data_pin;
	uint16_t current_button_state;
	uint16_t previous_button_state;
} snes_controller_t;

void snes_controller_init(snes_controller_t *snes_controller, TIM_HandleTypeDef *htim, GPIO_TypeDef *latch_port, uint16_t latch_pin, GPIO_TypeDef *clock_port, uint16_t clock_pin, GPIO_TypeDef *data_port, uint16_t data_pin);
void snes_controller_latch(snes_controller_t *snes_controller);
void snes_controller_clock(snes_controller_t *snes_controller);
void snes_controller_read(snes_controller_t *snes_controller);
void snes_controller_read2(snes_controller_t *snes_controller1, snes_controller_t *snes_controller2);

#endif /* INC_SNES_CONTROLLER_H_ */
