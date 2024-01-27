/*
 * game.c
 *
 *  Created on: Jan 26, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This file contains the logic to control the game.
 */

#include "game.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "splash.h"
#include "snake.h"
#include "menu.h"
#include "ws2812.h"
#include "led_grid.h"
#include "snes_controller.h"
#include "ring_buffer.h"
#include "ui.h"

// Global variables

uint16_t **grid_lookup;
uint8_t *brightness_lookup = NULL;
led_t led;

snes_controller_t controller1;
snes_controller_t controller2;
RingBuffer controller1_buffer;
RingBuffer controller2_buffer;

uint8_t update_screen_flag = 0;

// External variables
extern SPI_HandleTypeDef hspi1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim13;

void game_loop() {
    // Get controller states
    snes_controller_read2(&controller1, &controller2);

    // Show Game Splash Screen
    splash();

    /* Local Game variables */
    uint8_t game_over = 0;
    uint8_t game_reset = 0;
    uint8_t game_in_progress = 0;
    uint8_t game_pause = 0;
    uint16_t game_score[2] = { 0, 0 };
    uint16_t delay_counter = 0;
    uint8_t game_pace = 10;
    uint16_t best_score = 3;
    uint8_t game_level = 1;
    uint16_t apples_eaten[2] = { 0, 0 };
    uint8_t update_screen = 0;
    uint8_t display_contrast = 20;
    uint8_t level_up_interval = 5;
    uint8_t level_up_counter = 0;
    uint8_t order_of_play = 0;
    uint8_t perf_counter = 0;
    uint8_t death = 0;

    snake_status_t status = SNAKE_OK;
    snake_status_t status2 = SNAKE_OK;
    snake_status_t death_reason = SNAKE_DEATH_BY_WALL;
    snake_field_t *field;
//  snake_food_t food = { 0, 0 };
    snake_direction_t dir = SNAKE_NO_CHANGE;
    controller_direction_t dir_buffer;
    game_options_t game_options = { ONE_PLAYER, EASY, NO_POISON, 5 };

    WS2812_error_t led_error = WS2812_OK;
    uint16_t board_size = (GRID_SIZE * GRID_WIDTH) * (GRID_SIZE * GRID_HEIGHT);

    uint32_t start_time;
    uint32_t end_time;
    uint32_t elapsed_time;

    uint32_t game_start_time;
    uint32_t game_end_time;
    uint32_t game_elapsed_time = 0;

    uint8_t oled_buffer[20];

    /* Generate and initialize the grid lookup table */
    grid_lookup = generate_lookup_grid(GRID_SIZE * GRID_WIDTH, GRID_SIZE * GRID_HEIGHT, GRID_WIDTH,
    GRID_HEIGHT);

    /* Generate and initialize the brightness lookup table */
    brightness_lookup = generate_brightness_lookup_table(4);

    /* Initialize the LED grid */

    led_error = WS2812_init(&led, &htim3, TIM_CHANNEL_1, htim3.Init.Period, board_size, 0);
    if (led_error != WS2812_OK) {
        Error_Handler();
    }

    led.data_sent_flag = 1;

    WS2812_fill(&led, 0, 0, 32);
//    WS2812_set_brightness(&led, 5);
    WS2812_send(&led);

    osDelay(500);

    WS2812_clear(&led);
//    WS2812_set_brightness(&led, led.brightness);
    WS2812_send(&led);

    osDelay(10);

    // Grid test

    //grid_test(&led, 32, 16);

    // Initialize the ring buffer for controllers
    if (ring_buffer_init(&controller1_buffer, RING_BUFFER_SIZE, sizeof(controller_direction_t))
            == RING_BUFFER_MALLOC_FAILED) {
        Error_Handler();
    }
    if (ring_buffer_init(&controller2_buffer, RING_BUFFER_SIZE, sizeof(controller_direction_t))
            == RING_BUFFER_MALLOC_FAILED) {
        Error_Handler();
    }

    controller1.previous_button_state = 0xffff;
    controller2.previous_button_state = 0xffff;

    field = snake_field_init(GRID_SIZE * GRID_WIDTH, GRID_SIZE * GRID_HEIGHT);
    if (field == NULL) {
        Error_Handler();
    }

    ssd1306_Fill(Black);

    /* Infinite loop */
    for (;;) {
        if (!game_in_progress) {
            snes_controller_read2(&controller1, &controller2);
            if (!game_over) {

                if (controller1.current_button_state != controller1.previous_button_state) {

                    ssd1306_SetCursor(0, 3);
                    ssd1306_WriteString("Controller 1", Font_7x10, White);
                    snprintf((char*) oled_buffer, 16, "c: %04x p: %04x", controller1.current_button_state,
                            controller1.previous_button_state);
                    ssd1306_SetCursor(0, 15);
                    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
                    ssd1306_UpdateScreen();
                }
                if (controller2.current_button_state != controller2.previous_button_state) {
                    ssd1306_SetCursor(0, 32);
                    ssd1306_WriteString("Controller 2", Font_7x10, White);
                    snprintf((char*) oled_buffer, 16, "c: %04x p: %04x", controller2.current_button_state,
                            controller2.previous_button_state);
                    ssd1306_SetCursor(0, 42);
                    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
                    ssd1306_UpdateScreen();
                }
            } else if (game_over && !delay_counter) {
                ssd1306_Reset();
                ssd1306_Init();
                ssd1306_Fill(Black);
                ssd1306_SetCursor(14, 2);
                ssd1306_WriteString("GAME OVER", Font_11x18, White);
//                ssd1306_SetCursor(25, 54);
//                ssd1306_WriteString("Press Start", Font_7x10, White);
                delay_counter++;
            }
            if (game_over && delay_counter) {

                ui_game_over_screen(&game_options, game_score, best_score, delay_counter, game_level,
                        death_reason, apples_eaten, game_elapsed_time);
                delay_counter++;
                if (delay_counter > UI_DELAY * 3) {
                    delay_counter = 1;
                }
            }
            if (controller1.current_button_state == SNES_START_MASK) {

                menu_game_options(&game_options, &controller1);
                generate_wall(field, &game_options);
                game_in_progress = 1;

                // Set up common game variables
                game_score[0] = 0;
                game_score[1] = 0;
                delay_counter = 0;
                level_up_interval = 5;
                level_up_counter = 0;
                game_level = 1;
                apples_eaten[0] = 0;
                apples_eaten[1] = 0;
                death = 0;

                field->num_poisons_spawned = 0;
                field->poison_spawn_cooldown = 5;

                uint8_t start_x = field->width >> 1;
                uint8_t start_y = field->height >> 1;

                if (game_options.difficulty == EASY) {
                    field->max_poisons = game_options.poison ? 3 : 0;
                    field->poison_cooldown_base = 20;
                    game_pace = MAX_GAME_PACE;
                } else if (game_options.difficulty == MEDIUM) {
                    field->max_poisons = game_options.poison ? 5 : 0;
                    field->poison_cooldown_base = 15;
                    start_y = field->height - 3;
                    game_pace = MAX_GAME_PACE - GAME_PACE_STEP;
                } else if (game_options.difficulty == HARD) {
                    field->max_poisons = game_options.poison ? 7 : 0;
                    field->poison_cooldown_base = 10;
                    start_y = field->height - 3;
                    game_pace = MAX_GAME_PACE - (GAME_PACE_STEP * 2);
                } else if (game_options.difficulty == INSANE) {
                    field->max_poisons = game_options.poison ? 9 : 0;
                    field->poison_cooldown_base = 5;
                    start_y = field->height - 3;
                    game_pace = MAX_GAME_PACE - (GAME_PACE_STEP * 4);
                }

                // Set Up One Player Game Mode
                if (game_options.num_players == ONE_PLAYER) {
                    field->snake1 = snake_init(start_x, start_y);
                    if (field->snake1 == NULL) {
                        Error_Handler();
                    }
                    field->snake1->direction = SNAKE_UP;

                    status = snake_enqueue(field->snake1, start_x, start_y - 1);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = snake_enqueue(field->snake1, start_x, start_y - 2);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                    status = spawn_food(field);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    // Set Up Two Player Game Mode
                } else if (game_options.num_players == TWO_PLAYERS) {

                    field->snake1 = snake_init(start_x - 4, start_y);
                    if (field->snake1 == NULL) {
                        Error_Handler();
                    }
                    field->snake1->direction = SNAKE_UP;
                    field->snake1->color = GREEN;

                    field->snake2 = snake_init(start_x + 4, start_y);
                    if (field->snake2 == NULL) {
                        Error_Handler();
                    }
                    field->snake2->direction = SNAKE_UP;
                    field->snake2->color = BLUE;

                    status = snake_enqueue(field->snake1, start_x - 4, start_y - 1);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = snake_enqueue(field->snake2, start_x + 4, start_y - 1);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = snake_enqueue(field->snake1, start_x - 4, start_y - 2);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = snake_enqueue(field->snake2, start_x + 4, start_y - 2);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = spawn_food(field);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                }

                // Draw OLED screen
                ssd1306_Reset();
                ssd1306_Init();
                ssd1306_Fill(Black);

                if (game_options.num_players == ONE_PLAYER)
                    ui_one_player(game_score[0], best_score, game_level);
                else if (game_options.num_players == TWO_PLAYERS)
                    ui_two_player(game_score[0], game_score[1], game_level);

                game_start_time = __HAL_TIM_GET_COUNTER(&htim2);
            }
            osDelay(10);
        }

        if (game_in_progress) {
            start_time = __HAL_TIM_GET_COUNTER(&htim2);
            snes_controller_read2(&controller1, &controller2);
            if (controller1.current_button_state != controller1.previous_button_state
                    && controller1.current_button_state) {
                if (!game_pause) {
                    dir = snake_get_direction(&controller1, field->snake1->direction);
                    if (dir != SNAKE_NO_CHANGE && !is_ring_buffer_full(&controller1_buffer)) {
                        dir_buffer.current_direction = dir;
                        dir_buffer.previous_direction = field->snake1->direction;
                        ring_buffer_enqueue(&controller1_buffer, (void*) &dir_buffer);
                    }
                }
                if (controller1.current_button_state & SNES_START_MASK) {
                    game_pause = !game_pause;
                    update_screen = 1;
                } else if (controller1.current_button_state & SNES_SELECT_MASK) {
                    game_reset = 1;
                } else if (controller1.current_button_state & SNES_R_MASK) {
                    delay_counter = 0;
                    game_pace -= GAME_PACE_STEP;
                    if (game_pace <= GAME_PACE_STEP) {
                        game_pace = GAME_PACE_STEP;
                    }
                    update_screen = 1;
                } else if (controller1.current_button_state & SNES_L_MASK) {
                    delay_counter = 0;
                    game_pace += GAME_PACE_STEP;
                    if (game_pace >= MAX_GAME_PACE) {
                        game_pace = MAX_GAME_PACE;
                    }
                    update_screen = 1;
                } else if (controller1.current_button_state & SNES_A_MASK) {
                    display_contrast -= 5;
                    if (display_contrast == 0) {
                        display_contrast = 5;
                    }
                    ssd1306_SetContrast(display_contrast);
                } else if (controller1.current_button_state & SNES_X_MASK) {
                    display_contrast += 5;
                    if (display_contrast >= 250) {
                        display_contrast = 255;
                    }
                    ssd1306_SetContrast(display_contrast);
                }
            }
            if (controller2.current_button_state != controller2.previous_button_state
                    && controller2.current_button_state) {
                if (!game_pause && game_options.num_players == TWO_PLAYERS) {
                    dir = snake_get_direction(&controller2, field->snake2->direction);
                    if (dir != SNAKE_NO_CHANGE && !is_ring_buffer_full(&controller2_buffer)) {
                        dir_buffer.current_direction = dir;
                        dir_buffer.previous_direction = field->snake2->direction;
                        ring_buffer_enqueue(&controller2_buffer, (void*) &dir_buffer);
                    }
                }
            }
            if (delay_counter >= game_pace && !game_pause) {
                delay_counter = 0;
                order_of_play = !order_of_play;
                if (!is_ring_buffer_empty(&controller1_buffer)) {
                    ring_buffer_dequeue(&controller1_buffer, &dir_buffer);
                    field->snake1->direction = dir_buffer.current_direction;
                }
                if (!is_ring_buffer_empty(&controller2_buffer) && game_options.num_players == TWO_PLAYERS) {
                    ring_buffer_dequeue(&controller2_buffer, &dir_buffer);
                    field->snake2->direction = dir_buffer.current_direction;
                }

                if (game_options.num_players == TWO_PLAYERS) {
                    if (order_of_play) {
                        status = snake_move(field->snake1, field);
                        status2 = snake_move(field->snake2, field);
                    } else {
                        status2 = snake_move(field->snake2, field);
                        status = snake_move(field->snake1, field);
                    }
                } else {
                    status = snake_move(field->snake1, field);
                }

                if (status == SNAKE_FOOD_SPAWNED || status2 == SNAKE_FOOD_SPAWNED) {
                    spawn_food(field);
                    if (status == SNAKE_FOOD_SPAWNED) {
                        apples_eaten[0]++;
                        game_score[0] += game_options.difficulty + 1 + (game_level >> 1);
                    } else if (status2 == SNAKE_FOOD_SPAWNED) {
                        apples_eaten[1]++;
                        game_score[1] += game_options.difficulty + 1 + (game_level >> 1);
                    }

                    level_up_counter++;

                    if (game_options.num_players == ONE_PLAYER) {
                        if (game_score[0] > best_score) {
                            best_score = game_score[0];
                        }
                    }

                    if (level_up_counter >= level_up_interval) {
                        game_level++;
                        game_pace -= GAME_PACE_STEP;
                        if (game_pace <= GAME_PACE_STEP) {
                            game_pace = GAME_PACE_STEP;
                        }
                        level_up_interval += 5;
                        level_up_counter = 0;
                        if (game_pace == 0) {
                            game_pace = 1;
                        }
                        if (game_level % 2 == 1 && game_options.poison) {
                            field->max_poisons++;
                        }
                    }
                    update_screen = 1;
                }
                if (status == SNAKE_DEATH_BY_SELF || status == SNAKE_DEATH_BY_WALL
                        || status == SNAKE_DEATH_BY_POISON) {
                    death_reason = status;
                    death = 1;
                }
                if ((status2 == SNAKE_DEATH_BY_SELF || status2 == SNAKE_DEATH_BY_WALL
                        || status2 == SNAKE_DEATH_BY_POISON) && game_options.num_players == TWO_PLAYERS) {
                    death_reason = status2;
                    death = 1;
                }

                if (death) {
                    status = poison_food_destroy(field);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                    status = destroy_wall(field);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                    status = snake_destroy(field->snake1);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                    if (game_options.num_players == TWO_PLAYERS) {
                        status = snake_destroy(field->snake2);
                        if (status != SNAKE_OK) {
                            Error_Handler();
                        }
                    }
                    if (status == SNAKE_OK) {
                        field->snake1 = NULL;
                        field->snake2 = NULL;
                    }
                    game_over = 1;
                    game_in_progress = 0;
                    delay_counter = 0;
                    if (game_score[0] > best_score) {
                        best_score = game_score[0];
                    }
                    game_end_time = __HAL_TIM_GET_COUNTER(&htim2);
                    game_elapsed_time = (game_end_time - game_start_time) / 1000000;
                } else {
                    if (field->poison_spawn_cooldown > 0) {
                        field->poison_spawn_cooldown--;
                    }
                    poison_food_fade(field);
                    spawn_poison_food(field);
                    refresh_grid(&led, field, &game_options);

                }
            } else {
                delay_counter++;
            }
            if (update_screen) {

                if (game_options.num_players == ONE_PLAYER)
                    ui_one_player(game_score[0], best_score, game_level);
                else if (game_options.num_players == TWO_PLAYERS)
                    ui_two_player(game_score[0], game_score[1], game_level);

                if (game_pause) {
                    ssd1306_SetCursor(31, 24);
                    ssd1306_WriteString("PAUSED", Font_11x18, White);
                } else {
                    ssd1306_SetCursor(31, 24);
                    ssd1306_WriteString("       ", Font_11x18, White);
                }
            }

            end_time = __HAL_TIM_GET_COUNTER(&htim2);
            elapsed_time = end_time - start_time;
            game_elapsed_time = (end_time - game_start_time) / 1000000;

            if (!perf_counter || (update_screen && update_screen_flag)) {
                ui_latency_ms(elapsed_time);
                ui_elapsed_time(game_elapsed_time);
                perf_counter = 100;

                ssd1306_UpdateScreen();
                update_screen = 0;
            } else if (update_screen && update_screen_flag) {
                ssd1306_UpdateScreen();
                update_screen = 0;
            }
            perf_counter--;
            osDelay(3);
        }
    }
}
