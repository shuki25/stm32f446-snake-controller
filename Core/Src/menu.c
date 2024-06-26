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
#include "i2c_slave.h"

// Game options menu
const char *player_options[] = { "One Player", "Two Players" };
const char *level_options[] = { "Easy", "Medium", "Hard", "Insane" };
const char *poison_options[] = { "Poison Off", "Poison On" };

// Pause screen menu
const char *pause_options[] = { "Resume", "Restart", "Quit" };

// Settings screen menu
const char *settings_options[] = { "Brightness", "Clock", "Scoreboard", "Grid Size", "Reset" };

// Grid Size menu
const char *grid_size_options[] = { "16x16", "32x16", "32x32" };

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

void menu_game_options(game_options_t *options, snes_controller_t *controller1,
        snes_controller_t *controller2) {

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
        if (get_register_command()) { // Check for remote override
            return;
        }
        snes_controller_read2(controller1, controller2);
        if (controller1->current_button_state != controller1->previous_button_state
                && controller1->current_button_state) {
            if (controller1->current_button_state & SNES_UP_MASK) {
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
            } else if (controller1->current_button_state & SNES_DOWN_MASK) {
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
            } else if (controller1->current_button_state & SNES_LEFT_MASK) {
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
            } else if (controller1->current_button_state & SNES_RIGHT_MASK) {
                if (option_position == 0) {
                    if (options->num_players == 0 && controller2->is_active) {
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
            } else if (controller1->current_button_state & SNES_START_MASK
                    || controller1->current_button_state & SNES_B_MASK
                    || controller1->current_button_state & SNES_A_MASK) {
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
        if (get_register_command()) {  // Check for remote override
            return QUIT;
        }
        snes_controller_read(controller);
        if (controller->current_button_state != controller->previous_button_state
                && controller->current_button_state) {
            if (controller->current_button_state & SNES_X_MASK
                    || controller->current_button_state & SNES_Y_MASK) {
                return RESUME;
            }
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
            } else if (controller->current_button_state & SNES_B_MASK
                    || controller->current_button_state & SNES_A_MASK) {
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
    while (done < 2) {
        snes_controller_read(controller);
        if (controller->current_button_state == 0) {
            done++;
        }
        HAL_Delay(100);
    }

    done = 0;
    while (!done) {
        snes_controller_read(controller);

        if ((controller->current_button_state != controller->previous_button_state
                || (controller->current_button_state == controller->previous_button_state))
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
                } else if (cursor_value == max_value[column_position]) {
                    cursor_value = min_value[column_position];
                    blink_state = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_DOWN_MASK) {
                if (cursor_value > min_value[column_position]) {
                    cursor_value--;
                    blink_state = 1;
                    counter = 0;
                } else if (cursor_value == min_value[column_position]) {
                    cursor_value = max_value[column_position];
                    blink_state = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_Y_MASK) {
                return 0;
            } else if (controller->current_button_state & SNES_B_MASK) {
                value[column_position] = cursor_value;
                done = 1;
                break;
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

        osDelay(BLINK_DELAY * 3);
        counter--;
    }

    sDate->Year = value[0] - 2000;
    sDate->Month = value[1];
    sDate->Date = value[2];
    sTime->Hours = value[3];
    sTime->Minutes = value[4];
    if (value[5] + 7 < 60) {
        sTime->Seconds = value[5] + 7;
    } else {
        sTime->Seconds = 59;
    }
    sTime->Seconds = value[5];

    return 1;
}

void menu_redraw_options(const char *options[], uint8_t offset, uint8_t num_rows, uint8_t y_pos[]) {
    int j = 0;
    for (int i = offset; j < num_rows; i++, j++) {
        ssd1306_SetCursor(10, y_pos[j]);
        ssd1306_WriteString("               ", Font_7x10, White);
        uint8_t x_pos = center_text(options[i], Font_7x10);
        ssd1306_SetCursor(x_pos, y_pos[j]);
        ssd1306_WriteString((char*) options[i], Font_7x10, White);
    }
    ssd1306_UpdateScreen();
}

menu_settings_t menu_settings_screen(snes_controller_t *controller) {

    ssd1306_Fill(Black);
    ssd1306_SetCursor(20, 2);
    ssd1306_WriteString("Settings", Font_11x18, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);

    uint8_t done = 0;
    menu_pause_t option_position = 0;
    uint8_t blink_state = 1;
    uint8_t y_pos[3] = { 24, 38, 52 };
    uint8_t counter = 10;
    uint8_t scroll_size = 3;
    uint8_t scroll_position = 0;
    uint8_t scroll_max = NUM_MENU_SETTINGS_OPTIONS - 3;
    menu_settings_t adj_option_position = 0;

    menu_redraw_options(settings_options, scroll_position, scroll_size, y_pos);

//    for (int i = scroll_position; i < scroll_size; i++) {
//        uint8_t x_pos = center_text(settings_options[i], Font_7x10);
//        ssd1306_SetCursor(x_pos, y_pos[i]);
//        ssd1306_WriteString((char*) settings_options[i], Font_7x10, White);
//    }

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
                } else if (scroll_position > 0) {
                    scroll_position--;
                    menu_redraw_options(settings_options, scroll_position, scroll_size, y_pos);
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
                } else {
                    if (scroll_position < scroll_max) {
                        scroll_position++;
                        menu_redraw_options(settings_options, scroll_position, scroll_size, y_pos);
                    }
                }
            } else if (controller->current_button_state & SNES_B_MASK
                    || controller->current_button_state & SNES_A_MASK) {
                done = 1;
                done = 1;
            } else if (controller->current_button_state & SNES_Y_MASK
                    || controller->current_button_state & SNES_X_MASK) {
                option_position = NUM_MENU_SETTINGS_OPTIONS;
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
    adj_option_position = option_position + scroll_position;
    return adj_option_position;
}

void menu_player_initials(char *player_initials, uint16_t high_score, snes_controller_t *controller) {

    char buffer[20];

    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_SetCursor(9, 2);
    ssd1306_WriteString("High Score", Font_11x18, White);
    ssd1306_SetCursor(42, 20);
    sprintf(buffer, "%04d", high_score);
    ssd1306_WriteString(buffer, Font_11x18, White);
    ssd1306_SetCursor(53, 40);
    ssd1306_WriteString("___", Font_7x10, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);
    ssd1306_SetCursor(36, 53);
    ssd1306_WriteString("B = Save", Font_7x10, White);
    ssd1306_UpdateScreen();

    memset(player_initials, 0, 4);
    memset(player_initials, '_', 3);

    uint8_t is_done = 0;
    uint8_t initial_position = 0;
    uint8_t blink_state = 1;
    uint8_t counter = 3;
    uint8_t done = 0;

    // Wait til no buttons are pressed
    while (done < 2) {
        snes_controller_read(controller);
        if (controller->current_button_state == 0) {
            done++;
        }
        HAL_Delay(100);
    }
    while (!is_done) {
        snes_controller_read(controller);

        if ((controller->current_button_state != controller->previous_button_state
                || (controller->current_button_state == controller->previous_button_state))
                && controller->current_button_state) {
            if (controller->current_button_state & SNES_DOWN_MASK) {
                if (player_initials[initial_position] < 'Z') {
                    player_initials[initial_position]++;
                    blink_state = 1;
                    counter = 0;
                } else {
                    player_initials[initial_position] = 'A';
                    blink_state = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_UP_MASK) {
                if (player_initials[initial_position] > 'A') {
                    player_initials[initial_position]--;
                    blink_state = 1;
                    counter = 0;
                } else {
                    player_initials[initial_position] = 'Z';
                    blink_state = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_LEFT_MASK) {
                if (initial_position > 0) {
                    ssd1306_SetCursor(53 + (initial_position * 7), 40);
                    ssd1306_WriteString(&player_initials[initial_position], Font_7x10, White);
                    initial_position--;
                    blink_state = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_RIGHT_MASK) {
                if (initial_position < 2) {
                    ssd1306_SetCursor(53 + (initial_position * 7), 40);
                    ssd1306_WriteString(&player_initials[initial_position], Font_7x10, White);
                    initial_position++;
                    blink_state = 1;
                    counter = 0;
                }
            } else if (controller->current_button_state & SNES_B_MASK) {
                is_done = 1;
            }
        }

        if (blink_state && !counter) {
            ssd1306_SetCursor(53 + (initial_position * 7), 40);
            ssd1306_WriteString(&player_initials[initial_position], Font_7x10, White);
            blink_state = 0;
            counter = 3;
            ssd1306_UpdateScreen();
        } else if (!blink_state && !counter) {
            ssd1306_SetCursor(53 + (initial_position * 7), 40);
            ssd1306_WriteString(" ", Font_7x10, White);
            blink_state = 1;
            counter = 3;
            ssd1306_UpdateScreen();
        }
        counter--;
        osDelay(BLINK_DELAY * 3);
    }
}

void menu_scoreboard_settings(uint8_t *i2c_addr, snes_controller_t *controller) {

    ssd1306_Fill(Black);
    ssd1306_SetCursor(9, 2);
    ssd1306_WriteString("Scoreboard", Font_11x18, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);
    ssd1306_SetCursor(22, 22);
    ssd1306_WriteString("Press < > to", Font_7x10, White);
    ssd1306_SetCursor(8, 34);
    ssd1306_WriteString("assign I2C Addr.", Font_7x10, White);
    ssd1306_UpdateScreen();

    uint8_t current_i2c_addr = *i2c_addr;
    if (current_i2c_addr > 0x1F) {
        current_i2c_addr = 0x1F;
    } else if (current_i2c_addr < 0x10) {
        current_i2c_addr = 0x10;
    }

    char oled_buffer[20];
    memset(oled_buffer, 0, 20);

    uint8_t is_done = 0;
    while (!is_done) {
        snes_controller_read(controller);
        if (controller->current_button_state == 0) {
            is_done = 1;
        }
        osDelay(10);
    }

    sprintf(oled_buffer, "0x%02X", current_i2c_addr);
    ssd1306_SetCursor(50, 48);
    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
    ssd1306_UpdateScreen();

    is_done = 0;
    while (!is_done) {
        snes_controller_read(controller);
        if (controller->current_button_state == SNES_LEFT_MASK
                || controller->current_button_state == SNES_RIGHT_MASK) {
            if (controller->current_button_state == SNES_LEFT_MASK && current_i2c_addr > 0x10) {
                current_i2c_addr--;
            } else if (controller->current_button_state == SNES_RIGHT_MASK && current_i2c_addr < 0x1F) {
                current_i2c_addr++;
            }
            sprintf(oled_buffer, "0x%02X", current_i2c_addr);
            ssd1306_SetCursor(50, 48);
            ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
            ssd1306_UpdateScreen();
        } else if (controller->current_button_state == SNES_Y_MASK) {
            is_done = 1;
        } else if (controller->current_button_state == SNES_B_MASK) {
            *i2c_addr = current_i2c_addr;
            is_done = 1;
        }
        osDelay(100);
    }
}

void menu_grid_size_options(grid_size_options_t *grid_size, snes_controller_t *controller) {

    ssd1306_Fill(Black);
    ssd1306_SetCursor(14, 2);
    ssd1306_WriteString("Grid Size", Font_11x18, White);
    ssd1306_DrawRectangle(0, 0, 127, 63, White);

    uint8_t done = 0;
    uint8_t y_pos = 35;
    uint8_t blink_state = 1;
    uint8_t counter = 10;
    grid_size_options_t current_grid_size = *grid_size;
    uint8_t option_changed = 1;

    while (!done) {
        if (option_changed) {
            uint8_t x_pos = center_text(grid_size_options[current_grid_size], Font_7x10);
            ssd1306_SetCursor(x_pos, y_pos);
            ssd1306_WriteString((char*) grid_size_options[current_grid_size], Font_7x10, White);
            option_changed = 0;
        }

        snes_controller_read(controller);
        if (controller->current_button_state != controller->previous_button_state
                && controller->current_button_state) {
            if (controller->current_button_state & SNES_LEFT_MASK) {
                if (current_grid_size > 0) {
                    ssd1306_SetCursor(3, y_pos);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    ssd1306_SetCursor(117, y_pos);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    current_grid_size--;
                    counter = 0;
                    blink_state = 1;
                    option_changed = 1;
                }
            } else if (controller->current_button_state & SNES_RIGHT_MASK) {
                if (current_grid_size < NUM_GRID_SIZE_OPTIONS - 1) {
                    ssd1306_SetCursor(3, y_pos);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    ssd1306_SetCursor(117, y_pos);
                    ssd1306_WriteString(" ", Font_7x10, White);
                    current_grid_size++;
                    counter = 0;
                    blink_state = 1;
                    option_changed = 1;
                }
            } else if (controller->current_button_state & SNES_Y_MASK) {
                return;
            } else if (controller->current_button_state & SNES_B_MASK) {
                *grid_size = current_grid_size;
                ssd1306_SetCursor(3, y_pos);
                ssd1306_WriteString(" ", Font_7x10, White);
                ssd1306_SetCursor(117, y_pos);
                ssd1306_WriteString(" ", Font_7x10, White);
                ssd1306_SetCursor(4, y_pos);
                ssd1306_WriteString("                 ", Font_7x10, White);
                ssd1306_SetCursor(4, 27);
                ssd1306_WriteString("Grid Size Updated", Font_7x10, White);
                ssd1306_SetCursor(11, 42);
                ssd1306_WriteString("Reboot Required", Font_7x10, White);
                ssd1306_UpdateScreen();
                return;
            }
        }

        if (blink_state && !counter) {
            ssd1306_SetCursor(3, y_pos);
            ssd1306_WriteString(">", Font_7x10, White);
            ssd1306_SetCursor(117, y_pos);
            ssd1306_WriteString("<", Font_7x10, White);
            blink_state = 0;
            counter = 10;
            ssd1306_UpdateScreen();
        } else if (!blink_state && !counter) {
            ssd1306_SetCursor(3, y_pos);
            ssd1306_WriteString(" ", Font_7x10, White);
            ssd1306_SetCursor(117, y_pos);
            ssd1306_WriteString(" ", Font_7x10, White);
            blink_state = 1;
            counter = 10;
            ssd1306_UpdateScreen();
        }
        osDelay(BLINK_DELAY);
        counter--;
    }
}
