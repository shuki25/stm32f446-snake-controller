/*
 * game.h
 *
 *  Created on: Jan 26, 2024
 *      Author: josh
 */

#ifndef INC_GAME_H_
#define INC_GAME_H_

#include "main.h"

// ring buffer size
#define RING_BUFFER_SIZE 16
#define MAX_GAME_PACE 60
#define GAME_PACE_STEP 5

// Date Format
#define DATE_FORMAT "%04d-%02d-%02d"
#define TIME_FORMAT "%02d:%02d:%02d"

// EEPROM Defines
#define EEPROM_DATE_PAGE 0
#define EEPROM_DATE_OFFSET 0

void game_loop();


#endif /* INC_GAME_H_ */
