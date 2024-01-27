/*
 * menu.c
 *
 *  Created on: Jan 6, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This file contains the menu functions for the game.
 */

#include "menu.h"
#include <string.h>
#include "cmsis_os.h"

// Game options menu
const char *player_options[2] = { "One Player", "Two Players" };
const char *level_options[4] = { "Easy", "Medium", "Hard", "Insane" };
const char *poison_options[2] = { "Poison Off", "Poison On" };

// Pause screen menu
const char *pause_options[3] = { "Resume", "Restart", "Quit" };

uint8_t center_text(const char *text, FontDef font) {
    uint8_t text_length = strlen(text);
    return (uint8_t) ((128 - (text_length * font.FontWidth)) >> 1);
}

void clear_line(uint8_t width, uint8_t y_pos, FontDef font) {
    char blank[18];
    memset(blank, ' ', width);
    blank[width] = '\0';
    ssd1306_SetCursor(8, y_pos);
    ssd1306_WriteString(blank, font, White);
}

void menu_game_options(game_options_t *options, snes_controller_t *controller) {

    ssd1306_SetDisplayOn(0);
    ssd1306_Fill(Black);
    ssd1306_SetCursor(14, 2);
    ssd1306_WriteString("GAME MODE", Font_11x18, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);
    ssd1306_SetDisplayOn(1);

    uint8_t done = 0;
    uint8_t option_changed = 1;
    uint8_t option_position = 0;
    uint8_t blink_state = 1;
    uint8_t y_pos[3] = { 24, 38, 52 };
    uint8_t counter = 10;

    while (!done) {
        snes_controller_read(controller);
        if (controller->current_button_state != controller->previous_button_state
                && controller->current_button_state) {
            if (controller->current_button_state & SNES_UP_MASK) {
                if (option_position > 0) {
                    ssd1306_SetCursor(3, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    ssd1306_SetCursor(117, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    option_position--;
                    option_changed = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_DOWN_MASK) {
                if (options->difficulty == 3 && option_position == 1) {
                    // do nothing
                } else if (option_position < 2) {
                    ssd1306_SetCursor(3, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    ssd1306_SetCursor(117, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    option_position++;
                    option_changed = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_LEFT_MASK) {
                if (option_position == 0) {
                    if (options->num_players == 1) {
                        options->num_players = 0;
                        option_changed = 1;
                    }
                } else if (option_position == 1) {
                    if (options->difficulty == 3) {
                        options->poison = 0;
                    }
                    if (options->difficulty > 0) {
                        options->difficulty--;
                        option_changed = 1;
                    }

                } else if (option_position == 2) {
                    if (options->poison > 0) {
                        options->poison--;
                        option_changed = 1;
                    }
                }
            } else if (controller->current_button_state & SNES_RIGHT_MASK) {
                if (option_position == 0) {
                    if (options->num_players == 0) {
                        options->num_players = 1;
                        option_changed = 1;
                    }
                } else if (option_position == 1) {
                    if (options->difficulty < 3) {
                        options->difficulty++;
                        if (options->difficulty == 3) {
                            options->poison = 1;
                        }
                        option_changed = 1;
                    }

                } else if (option_position == 2) {
                    if (options->poison == 0) {
                        options->poison++;
                        option_changed = 1;
                    }
                }
            } else if (controller->current_button_state & SNES_START_MASK
                    || controller->current_button_state & SNES_A_MASK) {
                done = 1;
            }
        }

        if (option_changed) {
            uint8_t x_pos = center_text(player_options[options->num_players], Font_7x10);
            clear_line(16, y_pos[0], Font_7x10);
            ssd1306_SetCursor(x_pos, y_pos[0]);
            ssd1306_WriteString((char*) player_options[options->num_players], Font_7x10, White);

            x_pos = center_text(level_options[options->difficulty], Font_7x10);
            clear_line(16, y_pos[1], Font_7x10);
            ssd1306_SetCursor(x_pos, y_pos[1]);
            ssd1306_WriteString((char*) level_options[options->difficulty], Font_7x10, White);

            x_pos = center_text(poison_options[options->poison], Font_7x10);
            clear_line(16, y_pos[2], Font_7x10);
            ssd1306_SetCursor(x_pos, y_pos[2]);
            ssd1306_WriteString((char*) poison_options[options->poison], Font_7x10, White);
            option_changed = 0;
        }

        if (blink_state && !counter) {
            ssd1306_SetCursor(3, y_pos[option_position]);
            ssd1306_WriteString(">", Font_7x10, White);
            ssd1306_SetCursor(117, y_pos[option_position]);
            ssd1306_WriteString("<", Font_7x10, White);
            blink_state = 0;
            counter = 10;
            ssd1306_UpdateScreen();
        } else if (!blink_state && !counter) {
            ssd1306_SetCursor(3, y_pos[option_position]);
            ssd1306_WriteString(" ", Font_7x10, White);
            ssd1306_SetCursor(117, y_pos[option_position]);
            ssd1306_WriteString(" ", Font_7x10, White);
            blink_state = 1;
            counter = 10;
            ssd1306_UpdateScreen();
        }
        osDelay(BLINK_DELAY);
        counter--;
    }
}

menu_pause_t menu_pause_screen(snes_controller_t *controller) {
    ssd1306_SetDisplayOn(0);
    ssd1306_Fill(Black);
    ssd1306_SetCursor(31, 2);
    ssd1306_WriteString("PAUSED", Font_11x18, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);
    ssd1306_SetDisplayOn(1);

    uint8_t done = 0;
    menu_pause_t option_position = 0;
    uint8_t blink_state = 1;
    uint8_t y_pos[3] = { 24, 38, 52 };
    uint8_t counter = 10;

    for (int i = 0; i < NUM_MENU_PAUSE_OPTIONS; i++) {
        uint8_t x_pos = center_text(pause_options[i], Font_7x10);
        ssd1306_SetCursor(x_pos, y_pos[i]);
        ssd1306_WriteString((char*) pause_options[i], Font_7x10, White);
    }

    while (!done) {
        snes_controller_read(controller);
        if (controller->current_button_state != controller->previous_button_state
                && controller->current_button_state) {
            if (controller->current_button_state & SNES_UP_MASK) {
                if (option_position > 0) {
                    ssd1306_SetCursor(3, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    ssd1306_SetCursor(117, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    option_position--;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_DOWN_MASK) {
               if (option_position < 2) {
                    ssd1306_SetCursor(3, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    ssd1306_SetCursor(117, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    option_position++;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_A_MASK) {
                done = 1;
            }
        }

        if (blink_state && !counter) {
            ssd1306_SetCursor(3, y_pos[option_position]);
            ssd1306_WriteString(">", Font_7x10, White);
            ssd1306_SetCursor(117, y_pos[option_position]);
            ssd1306_WriteString("<", Font_7x10, White);
            blink_state = 0;
            counter = 10;
            ssd1306_UpdateScreen();
        } else if (!blink_state && !counter) {
            ssd1306_SetCursor(3, y_pos[option_position]);
            ssd1306_WriteString(" ", Font_7x10, White);
            ssd1306_SetCursor(117, y_pos[option_position]);
            ssd1306_WriteString(" ", Font_7x10, White);
            blink_state = 1;
            counter = 10;
            ssd1306_UpdateScreen();
        }

        osDelay(BLINK_DELAY);
        counter--;
    }
    return option_position;
}
