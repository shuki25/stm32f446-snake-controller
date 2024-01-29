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

#define BLINK_DELAY 50

typedef enum {
    RESUME, RESTART, QUIT, NUM_MENU_PAUSE_OPTIONS
} menu_pause_t;

typedef enum {
    BRIGHTNESS, SET_CLOCK, CLEAR_HIGH_SCORE, NUM_MENU_SETTINGS_OPTIONS
} menu_settings_t;

void menu_game_options(game_options_t *options, snes_controller_t *controller);
menu_pause_t menu_pause_screen(snes_controller_t *controller);
uint8_t menu_set_clock(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime, snes_controller_t *controller);
menu_settings_t menu_settings_screen(snes_controller_t *controller);

#endif /* INC_MENU_H_ */
