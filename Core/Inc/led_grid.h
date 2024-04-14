/*
 * led_grid.h
 *
 *  Created on: Jan 3, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *  Description: This header file defines the 2D array that represents the LED grid.
 *  The array is defined as a static array of hexadecimal values. The values are
 *  defined in the order that the LEDs are wired on the LED grid.
 *
 *  The LED grid is a 16x16 grid of LEDs. The LEDs are wired in a zig-zag pattern
 *  starting in the top left corner of the grid. The LEDs are wired in a row-major
 *  order. The first row of LEDs are wired from left to right. The second row of
 *  LEDs are wired from right to left. This pattern continues until the last row
 *  of LEDs are wired from left to right.
 *
 */

#ifndef INC_LED_GRID_H_
#define INC_LED_GRID_H_

// Define the dimensions of the LED grid
#define GRID_SIZE 16
#define GRID_WIDTH 2
#define GRID_HEIGHT 1
#include "stdint.h"
#include "ws2812.h"
#include "snake.h"

// Define the 2D array for the LED grid with hexadecimal values

// @formatter:off
//static uint8_t led_grid[GRID_SIZE][GRID_SIZE] = {
//	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
//	{31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16},
//	{32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47},
//	{63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48},
//	{64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79},
//	{95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80},
//	{96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111},
//	{127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112},
//	{128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143},
//	{159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144},
//	{160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175},
//	{191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176},
//	{192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207},
//	{223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208},
//	{224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239},
//	{255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240}
//};
// @formatter:on

extern uint16_t **grid_lookup;

uint16_t** generate_lookup_grid(uint8_t width, uint8_t height, uint8_t x_panel, uint8_t y_panel);
void destroy_lookup_grid(uint16_t **grid, uint8_t width);
void draw_snake(led_t *led_obj, snake_t *snake);
void draw_food(led_t *led_obj, snake_food_t *food);
void refresh_grid(led_t *led_obj, snake_field_t *field, game_options_t *options);
void grid_test(led_t *led_obj, uint8_t width, uint8_t height);
void grid_brightness_test(led_t *led_obj, uint8_t width, uint8_t height);
void grid_draw_font(led_t *led_obj, uint8_t width, uint8_t height, const char *text, uint8_t clear_grid,
        uint8_t color);
void grid_countdown(led_t *led_obj, uint8_t width, uint8_t height, uint8_t n, uint16_t delay_ms);

#endif /* INC_LED_GRID_H_ */
