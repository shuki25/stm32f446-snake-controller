/*
 * menu.h
 *
 *  Created on: Jan 6, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This file contains the menu functions for the game.
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#include <stdint.h>
#include "ssd1306.h"
#include "snes_controller.h"
#include "snake.h"

void menu_game_options(game_options_t *options, snes_controller_t *controller);

#endif /* INC_MENU_H_ */
