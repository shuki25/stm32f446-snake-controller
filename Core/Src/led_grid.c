/*
 * led_grid.c
 *
 *  Created on: Jan 4, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This file contains the functions to draw game objects on the WS2812B LED grid.
 */

#include "led_grid.h"
#include "cmsis_os.h"
#include "freeRTOS.h"

uint16_t** generate_lookup_grid(uint8_t width, uint8_t height, uint8_t x_panel, uint8_t y_panel) {
    uint16_t **grid = pvPortMalloc(width * sizeof(uint16_t*));
    if (grid == NULL) {
        return NULL;
    }

    for (int i = 0; i < width; i++) {
        grid[i] = pvPortMalloc(height * sizeof(uint16_t));
    }

    uint16_t led_pos = 0;
    uint16_t panel_count = 0;
    uint8_t x_panel_width = width / x_panel;
    uint8_t y_panel_height = height / y_panel;

    // Generate a lookup table for the LED grid (one panel is 16x16 LEDS arranged in x_panel by y_panel) and in zig-zag pattern

    for (int i = 0; i < x_panel; i++) {
        for (int j = 0; j < y_panel; j++) {
            for (int x = 0; x < width / x_panel; x++) {
                if (x % 2 == 0) {
                    for (int y = 0; y < height / y_panel; y++) {
                        grid[(x_panel_width * i) + x][(y_panel_height * j) + y] = led_pos;
                        led_pos++;
                    }
                } else {
                    for (int y = height / y_panel - 1; y >= 0; y--) {
                        grid[(x_panel_width * i) + x][(y_panel_height * j) + y] = led_pos;
                        led_pos++;
                    }
                }
            }
            panel_count++;
        }
    }

    return grid;
}

void destroy_lookup_grid(uint16_t **grid, uint8_t width) {
    for (int i = 0; i < width; i++) {
        vPortFree(grid[i]);
    }
    vPortFree(grid);
}

void draw_snake(led_t *led_obj, snake_t *snake) {
    snake_part_t *part = snake->head;
    while (part != NULL) {
        uint16_t led_pos = grid_lookup[part->x][part->y];
        if (part == snake->head) {
            WS2812_set_LED_color(led_obj, led_pos, YELLOW);
        } else {
            WS2812_set_LED_color(led_obj, led_pos, snake->color);
        }
        part = part->next;
    }
}

void draw_food(led_t *led_obj, snake_food_t *food) {
    uint16_t led_pos = grid_lookup[food->x][food->y];
    WS2812_set_LED_color(led_obj, led_pos, RED);
}

void draw_poison_food(led_t *led_obj, snake_poison_food_t *poison_food) {
    snake_poison_food_t *node = poison_food;

    while (node != NULL) {
        uint16_t led_pos = grid_lookup[node->x][node->y];
        WS2812_set_LED_color(led_obj, led_pos, MAGENTA);
        node = node->next;
    }
}

void draw_wall(led_t *led_obj, snake_wall_t *wall) {
    snake_wall_t *node = wall;

    while (node != NULL && node->visible) {
        uint16_t led_pos = grid_lookup[node->x][node->y];
        WS2812_set_LED_color(led_obj, led_pos, WHITE);
        node = node->next;
    }
}

void refresh_grid(led_t *led_obj, snake_field_t *field, game_options_t *options) {
    WS2812_clear(led_obj);
    draw_wall(led_obj, field->wall);
    draw_snake(led_obj, field->snake1);
    if (options->num_players == TWO_PLAYERS) {
        draw_snake(led_obj, field->snake2);
    }
    draw_food(led_obj, &field->food);
    draw_poison_food(led_obj, field->poison_food);
    WS2812_send(led_obj);
}

void grid_test(led_t *led_obj, uint8_t width, uint8_t height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            WS2812_clear(led_obj);
            uint16_t led_pos = grid_lookup[x][y];
            WS2812_set_LED_color(led_obj, led_pos, RED);
            WS2812_send(led_obj);
            osDelay(5);
        }
    }
}

void grid_brightness_test(led_t *led_obj, uint8_t width, uint8_t height) {

    uint16_t num_leds = width * height;
    uint16_t part_size = num_leds / 4;
    uint8_t color_increment = 255 / part_size;
    if (color_increment < 1) {
        color_increment = 1;
    }
    uint8_t color_group = 0;
    uint8_t counter = 0;
    uint16_t part_counter = 0;

    WS2812_clear(led_obj);

    for (int y = 0; y < height; y++) {
        if (part_counter >= part_size) {
            color_group++;
            counter = 0;
            part_counter = 0;
        }
        for (int x = 0; x < width; x++) {

            uint16_t led_pos = grid_lookup[x][y];
            if (color_group == 1) {
                WS2812_set_LED(led_obj, led_pos, counter, 0, 0);
            } else if (color_group == 2) {
                WS2812_set_LED(led_obj, led_pos, 0, counter, 0);
            } else if (color_group == 3) {
                WS2812_set_LED(led_obj, led_pos, 0, 0, counter);
            } else {
                WS2812_set_LED(led_obj, led_pos, counter, counter, counter);
            }
            counter += color_increment;
            if (counter > 255) {
                counter = 255;
            }
            part_counter++;
        }
    }
    WS2812_send(led_obj);
}

