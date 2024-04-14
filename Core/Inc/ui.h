/*
 * ui.h
 *
 *  Created on: Jan 20, 2024
 *      Author: Joshua Butler, MD, MHI
 */

#ifndef INC_UI_H_
#define INC_UI_H_

#include "snake.h"

#define UI_DELAY 125

void ui_one_player(uint16_t game_score, uint16_t best_score, uint8_t game_level);
void ui_two_player(uint16_t p1_score, uint16_t p2_score, uint8_t game_level);
void ui_forced_end_game_screen();
void ui_prepare_game_screen();
void ui_game_over_screen(game_options_t *options, uint16_t *game_score, uint16_t best_score,
        uint16_t delay_counter, uint8_t game_level, uint8_t death_reason, uint16_t *apples_eaten,
        uint32_t game_elapsed_time, char *best_score_name);
void ui_elapsed_time(uint32_t game_elapsed_time);
void ui_latency_ms(uint32_t latency);

#endif /* INC_UI_H_ */
