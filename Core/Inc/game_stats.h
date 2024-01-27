/*
 * game_stats.h
 *
 *  Created on: Jan 26, 2024
 *      Author: Joshua Butler, MD, MHI
 */

#ifndef INC_GAME_STATS_H_
#define INC_GAME_STATS_H_

#include <stdint.h>
#include "snake.h"
#include "cmsis_os.h"


/*
 * Data structure for storing game statistics
 *
 * difficulty: game difficulty
 * poison: poison status
 * high_score: high score
 * level: current level
 * num_apples_eaten: number of apples eaten
 * length_played: length of time played
 * sDate: date of game
 * player_name: player name
 * times_played: number of games played at this difficulty
 *
 * Total size of struct must be less than 32 bytes as the EEPROM page size is 32 bytes.
 * The current struct size is 24 bytes. If the struct size is changed, the EEPROM
 * must be erased and the data will get reset.
 *
 */

typedef struct game_stats {
    options_difficulty_t difficulty;
    options_poison_t poison;
    uint16_t high_score;
    uint8_t level;
    uint16_t num_apples_eaten;
    uint32_t length_played;
    RTC_DateTypeDef sDate;
    char player_name[4];
    uint16_t times_played;
} game_stats_t;

#endif /* INC_GAME_STATS_H_ */
