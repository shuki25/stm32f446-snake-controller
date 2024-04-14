/*
 * snake.h
 *
 *  Created on: Jan 3, 2024
 *      Author: josh
 */

#ifndef INC_SNAKE_H_
#define INC_SNAKE_H_

#include <stdint.h>
#include "ws2812.h"
#include "snes_controller.h"

typedef struct snakeFood {
	uint8_t x;
	uint8_t y;
} snake_food_t;

typedef struct snakePoisonFood {
	uint8_t x;
	uint8_t y;
	uint8_t duration;
	struct snakePoisonFood *next;
	struct snakePoisonFood *prev;
} snake_poison_food_t;

typedef struct snakeWall {
	uint8_t x;
	uint8_t y;
	uint8_t visible;
	struct snakeWall *next;
	struct snakeWall *prev;
} snake_wall_t;

typedef struct snakePart {
	uint8_t x;
	uint8_t y;
	struct snakePart *next;
	struct snakePart *prev;
} snake_part_t;

typedef enum {
	SNAKE_UP, SNAKE_DOWN, SNAKE_LEFT, SNAKE_RIGHT, SNAKE_NO_CHANGE
} snake_direction_t;

typedef struct Snake {
	snake_part_t *head;
	snake_part_t *tail;
	WS2812_color_t color;
	uint8_t length;
	snake_direction_t direction;
} snake_t;

typedef struct snakeField {
	uint8_t width;
	uint8_t height;
	snake_t *snake1;
	snake_t *snake2;
	snake_food_t food;
	snake_poison_food_t *poison_food;
	uint8_t poison_spawn_cooldown;
	uint8_t poison_cooldown_base;
	uint8_t num_poisons_spawned;
	uint8_t max_poisons;
	snake_wall_t *wall;
} snake_field_t;

typedef enum {
	SNAKE_OK,
	SNAKE_FOOD_SPAWNED,
	SNAKE_OUT_OF_BOUNDS,
	SNAKE_DEATH_BY_WALL,
	SNAKE_DEATH_BY_BODY,
	SNAKE_DEATH_BY_SELF,
	SNAKE_DEATH_BY_POISON,
	SNAKE_WALL_NOT_GENERATED,
	SNAKE_WALL_GENERATED,
	SNAKE_ERROR,
	SNAKE_MALLOC_FAILED
} snake_status_t;

typedef enum {
	SINGLE_PLAYER, TWO_PLAYERS_SNAKE, TWO_PLAYERS_FOOD
} snake_game_mode_t;

typedef enum {
	NO_COLLISION, SNAKE_BODY_COLLISION, FOOD_COLLISION, POISON_FOOD_COLLISION, WALL_COLLISION, SNAKE_SELF_COLLISION
} snake_collision_t;

typedef struct controller_direction_t {
    snake_direction_t current_direction;
    snake_direction_t previous_direction;
} controller_direction_t;

typedef enum {
	ONE_PLAYER, TWO_PLAYERS
} options_num_players_t;

typedef enum {
	EASY, MEDIUM, HARD, INSANE, NUM_DIFFICULTIES
} options_difficulty_t;

typedef enum {
	NO_POISON, POISON
} options_poison_t;

typedef struct {
	options_num_players_t num_players;
	options_difficulty_t difficulty;
	options_poison_t poison;
	uint8_t num_poisons;
	uint8_t can_change_speed;
} game_options_t;

snake_field_t* snake_field_init(uint8_t width, uint8_t height);
snake_status_t snake_field_destroy(snake_field_t *field);
snake_t *snake_init(uint8_t x, uint8_t y);
snake_status_t snake_enqueue(snake_t *snake, uint8_t x, uint8_t y);
snake_status_t snake_dequeue(snake_t *snake);
snake_status_t snake_destroy(snake_t *snake);
uint8_t snake_contains(snake_t *snake, uint8_t x, uint8_t y);
uint8_t poison_food_contains(snake_field_t *field, uint8_t x, uint8_t y);
snake_collision_t check_collision(snake_field_t *field, uint8_t x, uint8_t y);
snake_wall_t *add_wall_node(snake_wall_t *node, uint8_t x, uint8_t y);
snake_status_t generate_wall(snake_field_t *field, game_options_t *game_options);
snake_status_t destroy_wall(snake_field_t *field);
snake_status_t spawn_poison_food(snake_field_t *field);
snake_status_t poison_food_destroy(snake_field_t *field);
snake_status_t poison_food_fade(snake_field_t *field);
snake_status_t spawn_food(snake_field_t *field);
snake_status_t snake_move(snake_t *snake, snake_field_t *field);
snake_direction_t snake_get_direction(snes_controller_t *snes_controller, snake_direction_t current_direction);

#endif /* INC_SNAKE_H_ */
