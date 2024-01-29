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
#include <stdio.h>
#include "cmsis_os.h"
#include "bcd_util.h"

// Game options menu
const char *player_options[2] = { "One Player", "Two Players" };
const char *level_options[4] = { "Easy", "Medium", "Hard", "Insane" };
const char *poison_options[2] = { "Poison Off", "Poison On" };

// Pause screen menu
const char *pause_options[3] = { "Resume", "Restart", "Quit" };

// Settings screen menu
const char *settings_options[3] = { "Brightness", "Clock", "Reset" };

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

    ssd1306_Fill(Black);
    ssd1306_SetCursor(14, 2);
    ssd1306_WriteString("GAME MODE", Font_11x18, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);

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
                    blink_state = 1;
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
                    blink_state = 1;
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
                    blink_state = 1;
                }
            } else if (controller->current_button_state & SNES_DOWN_MASK) {
                if (option_position < 2) {
                    ssd1306_SetCursor(3, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    ssd1306_SetCursor(117, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    option_position++;
                    counter = 0;
                    blink_state = 1;
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

uint8_t menu_set_clock(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime, snes_controller_t *controller) {
    ssd1306_Fill(Black);
    ssd1306_SetCursor(14, 2);
    ssd1306_WriteString("Date/Time", Font_11x18, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);

    uint8_t done = 0;
    uint8_t position_changed = 1;
    uint8_t option_position = 0;
    uint8_t column_position = 0;
    uint8_t blink_state = 1;
    uint8_t x_pos[6] = { 25, 60, 81, 36, 57, 78 };
    uint8_t y_pos[2] = { 24, 38 };
    uint8_t max_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 30, 30, 31 };
    uint16_t cursor_value = 0;
    uint8_t counter = 3;
    uint16_t value[6] = { 0, 0, 0, 0, 0, 0 };
    uint16_t max_value[6] = { 2099, 12, 31, 23, 59, 59 };
    uint16_t min_value[6] = { 2024, 1, 1, 0, 0, 0 };
    char buffer[20];

    if (sDate->Year % 4 == 0 || sDate->Year % 100 == 0 || sDate->Year % 400 == 0) {
        max_days[1] = 29;
    } else {
        max_days[1] = 28;
    }

    value[0] = sDate->Year + 2000;
    value[1] = sDate->Month;
    value[2] = sDate->Date;
    value[3] = sTime->Hours;
    value[4] = sTime->Minutes;
    value[5] = sTime->Seconds;

    cursor_value = sDate->Year + 2000;

    // Wait til no buttons are pressed
    while (!done) {
        snes_controller_read(controller);
        if (controller->current_button_state == 0) {
            done = 1;
        }
    }

    done = 0;
    while (!done) {
        snes_controller_read(controller);

        if ((controller->current_button_state != controller->previous_button_state || (controller->current_button_state == controller->previous_button_state))
                && controller->current_button_state) {
            if (controller->current_button_state & SNES_LEFT_MASK) {
                if (column_position > 0) {
                    value[column_position] = cursor_value;
                    if (column_position == 0) { // Fix leap day if necessary
                        if (cursor_value % 4 == 0 || cursor_value % 100 == 0 || cursor_value % 400 == 0) {
                            max_days[1] = 29;
                        } else {
                            max_days[1] = 28;
                        }
                    }

                    if (column_position == 0 || column_position == 1) {
                        max_value[2] = max_days[value[1] - 1];
                        if (value[2] > max_value[2]) {
                            value[2] = max_value[2];
                        }
                    }

                    column_position--;
                    position_changed = 1;
                    cursor_value = value[column_position];

                    if (column_position < 3) {
                        option_position = 0;
                    } else {
                        option_position = 1;
                    }
                    blink_state = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_RIGHT_MASK) {
                if (column_position < 6) {
                    value[column_position] = cursor_value;

                    if (column_position == 0) { // Fix leap day if year changes
                        if (cursor_value % 4 == 0 || cursor_value % 100 == 0 || cursor_value % 400 == 0) {
                            max_days[1] = 29;
                        } else {
                            max_days[1] = 28;
                        }
                    }
                    if (column_position == 0 || column_position == 1) {
                        max_value[2] = max_days[value[1] - 1];
                        if (value[2] > max_value[2]) {
                            value[2] = max_value[2];
                        }
                    }

                    column_position++;
                    position_changed = 1;
                    cursor_value = value[column_position];
                    if (column_position < 3) {
                        option_position = 0;
                    } else {
                        option_position = 1;
                    }
                    blink_state = 1;
                    counter = 0;
                }

            } else if (controller->current_button_state & SNES_UP_MASK) {
                if (cursor_value < max_value[column_position]) {
                    cursor_value++;
                    blink_state = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_DOWN_MASK) {
                if (cursor_value > min_value[column_position]) {
                    cursor_value--;
                    blink_state = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_Y_MASK) {
               return 0;
            } else if (controller->current_button_state & SNES_B_MASK) {
                value[column_position] = cursor_value;
                done = 1;
            }
        }

        if (position_changed) {
            // Display current date and time
            ssd1306_SetCursor(x_pos[0], y_pos[0]);
            sprintf(buffer, "%04d/%02d/%02d", value[0], value[1], value[2]);
            ssd1306_WriteString(buffer, Font_7x10, White);

            ssd1306_SetCursor(x_pos[3], y_pos[1]);
            sprintf(buffer, "%02d:%02d:%02d", value[3], value[4], value[5]);
            ssd1306_WriteString(buffer, Font_7x10, White);

            ssd1306_SetCursor(3, y_pos[!option_position]);
            ssd1306_WriteString(" ", Font_7x10, White);
            ssd1306_SetCursor(117, y_pos[!option_position]);
            ssd1306_WriteString(" ", Font_7x10, White);
            ssd1306_SetCursor(3, y_pos[option_position]);
            ssd1306_WriteString(">", Font_7x10, White);
            ssd1306_SetCursor(117, y_pos[option_position]);
            ssd1306_WriteString("<", Font_7x10, White);
            ssd1306_UpdateScreen();

            position_changed = 0;
        }
        if (blink_state && !counter) {
            ssd1306_SetCursor(x_pos[column_position], y_pos[option_position]);
            if (column_position == 0) {
                sprintf(buffer, "%04d", cursor_value);
            } else {
                sprintf(buffer, "%02d", cursor_value);
            }
            ssd1306_WriteString(buffer, Font_7x10, White);
            blink_state = 0;
            counter = 3;
            ssd1306_UpdateScreen();
        } else if (!blink_state && !counter) {
            ssd1306_SetCursor(x_pos[column_position], y_pos[option_position]);
            if (column_position == 0) {
                ssd1306_WriteString("    ", Font_7x10, White);
            } else {
                ssd1306_WriteString("  ", Font_7x10, White);
            }

            blink_state = 1;
            counter = 3;
            ssd1306_UpdateScreen();
        }

        osDelay(BLINK_DELAY*3);
        counter--;
    }

    sDate->Year = value[0] - 2000;
    sDate->Month = value[1];
    sDate->Date = value[2];
    sTime->Hours = value[3];
    sTime->Minutes = value[4];
    sTime->Seconds = value[5];

    return 1;
}

menu_settings_t menu_settings_screen(snes_controller_t *controller) {
    ssd1306_SetDisplayOn(0);
    ssd1306_Fill(Black);
    ssd1306_SetCursor(20, 2);
    ssd1306_WriteString("Settings", Font_11x18, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);
    ssd1306_SetDisplayOn(1);

    uint8_t done = 0;
    menu_pause_t option_position = 0;
    uint8_t blink_state = 1;
    uint8_t y_pos[3] = { 24, 38, 52 };
    uint8_t counter = 10;

    for (int i = 0; i < NUM_MENU_SETTINGS_OPTIONS; i++) {
        uint8_t x_pos = center_text(settings_options[i], Font_7x10);
        ssd1306_SetCursor(x_pos, y_pos[i]);
        ssd1306_WriteString((char*) settings_options[i], Font_7x10, White);
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
                    blink_state = 1;
                }
            } else if (controller->current_button_state & SNES_DOWN_MASK) {
                if (option_position < 2) {
                    ssd1306_SetCursor(3, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    ssd1306_SetCursor(117, y_pos[option_position]);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    option_position++;
                    counter = 0;
                    blink_state = 1;
                }
            } else if (controller->current_button_state & SNES_B_MASK) {
                done = 1;
            } else if (controller->current_button_state & SNES_Y_MASK) {
                option_position = 3;
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


//void menu_player_initials(char *player_initials) {
//    ssd1306_Fill(Black);
//    ssd1306_SetCursor(14, 2);
//    ssd1306_WriteString("PLAYER INITIALS", Font_11x18, White);
//    ssd1306_DrawRectangle(0, 0, 127, 63, White);
//
//    uint8_t done = 0;
//    uint8_t option_changed = 1;
//    uint8_t option_position = 0;
//    uint8_t blink_state = 1;
//    uint8_t y_pos[3] = { 24, 38, 52 };
//    uint8_t counter = 10;
//
//    while (!done) {
//        if (option_changed) {
//            uint8_t x_pos = center_text(player_initials, Font_7x10);
//            clear_line(16, y_pos[0], Font_7x10);
//            ssd1306_SetCursor(x_pos, y_pos[0]);
//            ssd1306_WriteString((char*) player_initials, Font_7x10, White);
//            option_changed = 0;
//        }
//
//        snes_controller_t controller;
//        snes_controller_read(&controller);
//    }
//}
