/*
 * ui.c
 *
 *  Created on: Jan 20, 2024
 *      Author: Joshua Butler, MD, MHI
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ui.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

void ui_one_player(uint16_t game_score, uint16_t best_score, uint8_t game_level, uint8_t tournament_mode) {

    uint8_t oled_buffer[20];

    ssd1306_SetCursor(4, 2);
    ssd1306_WriteString("P1", Font_7x10, White);
    ssd1306_SetCursor(53, 2);
    ssd1306_WriteString("LVL", Font_7x10, White);
    ssd1306_SetCursor(96, 2);
    ssd1306_WriteString("Best", Font_7x10, White);

    snprintf((char*) oled_buffer, 16, "%d", game_score);
    ssd1306_SetCursor(4, 14);
    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);

    snprintf((char*) oled_buffer, 16, "%d", game_level);
    ssd1306_SetCursor(53, 14);
    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);

    snprintf((char*) oled_buffer, 16, "%d", best_score);
    uint8_t len = strlen((char*) oled_buffer);
    ssd1306_SetCursor((124 - (len * 7)), 14);
    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);

    if (tournament_mode) {
        ssd1306_SetCursor(29, 32);
        ssd1306_WriteString("Tournament", Font_7x10, White);
    }
}

void ui_two_player(uint16_t p1_apples, uint16_t p2_apples, uint8_t game_level) {

    uint8_t oled_buffer[20];

    ssd1306_SetCursor(4, 2);
    ssd1306_WriteString("P1", Font_7x10, White);
    ssd1306_SetCursor(53, 2);
    ssd1306_WriteString("LVL", Font_7x10, White);
    ssd1306_SetCursor(112, 2);
    ssd1306_WriteString("P2", Font_7x10, White);

    snprintf((char*) oled_buffer, 16, "%d", p1_apples); // Player 1 Number of Apples Eaten
    ssd1306_SetCursor(4, 14);
    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);

    snprintf((char*) oled_buffer, 16, "%d", game_level);
    ssd1306_SetCursor(53, 14);
    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);

    snprintf((char*) oled_buffer, 16, "%d", p2_apples); // Player 2 Number of Apples Eaten
    uint8_t len = strlen((char*) oled_buffer);
    ssd1306_SetCursor((128 - (len * 7)), 14);
    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
}

void ui_forced_end_game_screen() {
    ssd1306_Fill(Black);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);
    ssd1306_SetCursor(14, 5);
    ssd1306_WriteString("GAME OVER", Font_11x18, White);
    ssd1306_SetCursor(19, 31);
    ssd1306_WriteString("Game Ended by", Font_7x10, White);
    ssd1306_SetCursor(33, 43);
    ssd1306_WriteString("Moderator", Font_7x10, White);
    ssd1306_UpdateScreen();
}

void ui_prepare_game_screen() {
    ssd1306_Fill(Black);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);
    ssd1306_SetCursor(29, 12);
    ssd1306_WriteString("Starting a", Font_7x10, White);
    ssd1306_SetCursor(29, 24);
    ssd1306_WriteString("Tournament", Font_7x10, White);
    ssd1306_SetCursor(29, 41);
    ssd1306_WriteString("Get Ready!", Font_7x10, White);
    ssd1306_UpdateScreen();
}

void ui_wait_end_tournament_screen() {
    ssd1306_Fill(Black);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);
    ssd1306_SetCursor(25, 14);
    ssd1306_WriteString("Waiting for", Font_7x10, White);
    ssd1306_SetCursor(15, 26);
    ssd1306_WriteString("the tournament", Font_7x10, White);
    ssd1306_SetCursor(25, 38);
    ssd1306_WriteString("to conclude", Font_7x10, White);
    ssd1306_UpdateScreen();
}

void ui_game_over_screen(game_options_t *options, uint16_t *game_score, uint16_t best_score,
        uint16_t delay_counter, uint8_t game_level, uint8_t death_reason, uint16_t *apples_eaten,
        uint32_t game_elapsed_time, char *best_score_name) {

    uint8_t oled_buffer[24];
    uint8_t len;

    ssd1306_SetCursor(14, 0);
    ssd1306_WriteString("GAME OVER", Font_11x18, White);

    if (options->num_players == ONE_PLAYER) {
        if (delay_counter <= 1) {
            ssd1306_SetCursor(1, 26);
            ssd1306_WriteString("                ", Font_7x10, White);

            snprintf((char*) oled_buffer, 24, "Score: %d (L.%d)", game_score[0], game_level);
            len = strlen((char*) oled_buffer);
            ssd1306_SetCursor((128 - (7 * len)) >> 1, 26);
            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);

            ssd1306_SetCursor(1, 38);
            ssd1306_WriteString("                ", Font_7x10, White);

            snprintf((char*) oled_buffer, 24, "Best: %d (%s)", best_score, best_score_name);
            len = strlen((char*) oled_buffer);
            ssd1306_SetCursor((128 - (7 * len)) >> 1, 38);
            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
            ssd1306_UpdateScreen();
        } else if (delay_counter == UI_DELAY) {
            ssd1306_SetCursor(1, 26);
            ssd1306_WriteString("     Death by     ", Font_7x10, White);
            ssd1306_SetCursor(1, 38);
            switch (death_reason) {
                case SNAKE_DEATH_BY_WALL:
                    ssd1306_WriteString("     the Wall     ", Font_7x10, White);
                    break;
                case SNAKE_DEATH_BY_BODY:
                case SNAKE_DEATH_BY_SELF:
                    ssd1306_WriteString("    Snake Bite    ", Font_7x10, White);
                    break;
                case SNAKE_DEATH_BY_POISON:
                    ssd1306_WriteString("      Poison      ", Font_7x10, White);
                    break;
                default:
                    ssd1306_WriteString("  Unknown Death  ", Font_7x10, White);
                    break;
            }
            ssd1306_UpdateScreen();
        } else if (delay_counter == UI_DELAY * 2) {
            ssd1306_SetCursor(1, 26);
            ssd1306_WriteString("                ", Font_7x10, White);

            snprintf((char*) oled_buffer, 17, "Apples: %d", apples_eaten[0]);
            len = strlen((char*) oled_buffer);
            ssd1306_SetCursor((128 - (7 * len)) >> 1, 26);
            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);

            ssd1306_SetCursor(1, 38);
            ssd1306_WriteString("                ", Font_7x10, White);
            snprintf((char*) oled_buffer, 17, "Time: %d:%02d", (uint8_t) (game_elapsed_time / 60),
                    (uint8_t) (game_elapsed_time % 60));
            len = strlen((char*) oled_buffer);
            ssd1306_SetCursor((128 - (7 * len)) >> 1, 38);
            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
            ssd1306_UpdateScreen();
        }
    } else {
//        if (delay_counter <= 1) {
//            ssd1306_SetCursor(1, 26);
//            ssd1306_WriteString("                ", Font_7x10, White);
//
//            snprintf((char*) oled_buffer, 24, "P1 Score: %d", game_score[0]);
//            len = strlen((char*) oled_buffer);
//            ssd1306_SetCursor((128 - (7 * len)) >> 1, 26);
//            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
//
//            ssd1306_SetCursor(1, 38);
//            ssd1306_WriteString("                ", Font_7x10, White);
//
//            snprintf((char*) oled_buffer, 16, "P2 Score: %d", game_score[1]);
//            len = strlen((char*) oled_buffer);
//            ssd1306_SetCursor((128 - (7 * len)) >> 1, 38);
//            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
//            ssd1306_UpdateScreen();
//        }
        if (delay_counter <= 1) {
            ssd1306_SetCursor(1, 26);
            ssd1306_WriteString("     Death by     ", Font_7x10, White);

            ssd1306_SetCursor(1, 50);
            ssd1306_WriteString("                ", Font_7x10, White); // Clear the time from previous screen

            ssd1306_SetCursor(1, 38);
            switch (death_reason) {
                case SNAKE_DEATH_BY_WALL:
                    ssd1306_WriteString("     the Wall     ", Font_7x10, White);
                    break;
                case SNAKE_DEATH_BY_BODY:
                case SNAKE_DEATH_BY_SELF:
                    ssd1306_WriteString("    Snake Bite    ", Font_7x10, White);
                    break;
                case SNAKE_DEATH_BY_POISON:
                    ssd1306_WriteString("      Poison      ", Font_7x10, White);
                    break;
                default:
                    ssd1306_WriteString("  Unknown Death  ", Font_7x10, White);
                    break;
            }
            ssd1306_UpdateScreen();
        } else if (delay_counter == UI_DELAY) {
            ssd1306_SetCursor(1, 26);
            ssd1306_WriteString("                ", Font_7x10, White);

            snprintf((char*) oled_buffer, 17, "P1 Apples: %d", apples_eaten[0]);
            len = strlen((char*) oled_buffer);
            ssd1306_SetCursor((128 - (7 * len)) >> 1, 26);
            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);

            snprintf((char*) oled_buffer, 17, "P2 Apples: %d", apples_eaten[1]);
            len = strlen((char*) oled_buffer);
            ssd1306_SetCursor((128 - (7 * len)) >> 1, 38);
            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);

            snprintf((char*) oled_buffer, 17, "Time: %d:%02d", (uint8_t) (game_elapsed_time / 60),
                    (uint8_t) (game_elapsed_time % 60));
            len = strlen((char*) oled_buffer);
            ssd1306_SetCursor((128 - (7 * len)) >> 1, 50);
            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
            ssd1306_UpdateScreen();
        }

    }

}

void ui_elapsed_time(uint32_t game_elapsed_time) {
    uint8_t oled_buffer[16];
    uint8_t len;

    snprintf((char*) oled_buffer, 16, "%d:%02d", (uint8_t) (game_elapsed_time / 60),
            (uint8_t) (game_elapsed_time % 60));
    len = strlen((char*) oled_buffer);
    ssd1306_SetCursor((128 - (7 * len)), 54);
    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
    ssd1306_UpdateScreen();
}

void ui_latency_ms(uint32_t latency) {
    uint8_t oled_buffer[16];

    snprintf((char*) oled_buffer, 16, " %3lu.%0lu ms", (uint32_t) latency / 1000, latency % 10);
    ssd1306_SetCursor(0, 54);
    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
    ssd1306_UpdateScreen();
}
