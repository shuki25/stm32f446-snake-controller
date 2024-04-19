/*
 * snake.c
 *
 *  Created on: Jan 3, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *     This is the main snake game file. It contains the game logic and the game loop.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "freertos.h"
#include "snake.h"
#include "rng.h"

extern uint8_t is_random_seed_shared;

// Initialize the playing field

snake_field_t* snake_field_init(uint8_t width, uint8_t height) {
	snake_field_t *field = (snake_field_t*) pvPortMalloc(sizeof(snake_field_t));
	if (field == NULL) {
		return NULL;
	}
	field->width = width;
	field->height = height;
	field->snake1 = NULL;
	field->snake2 = NULL;
	field->food.x = 0;
	field->food.y = 0;
	field->poison_food = NULL;
	field->num_poisons_spawned = 0;
	field->max_poisons = 0;
	field->wall = NULL;

	return field;
}

// Destroy the playing field

snake_status_t snake_field_destroy(snake_field_t *field) {
    snake_destroy(field->snake1);
    snake_destroy(field->snake2);
    poison_food_destroy(field);
    destroy_wall(field);
    vPortFree(field);
    return SNAKE_OK;
}

// Function to handle snake position and movement

snake_t* snake_init(uint8_t x, uint8_t y) {

	snake_t *snake = (snake_t*) pvPortMalloc(sizeof(snake_t));
	if (snake == NULL) {
		return NULL;
	}
	snake->head = (snake_part_t*) pvPortMalloc(sizeof(snake_part_t));
	if (snake->head == NULL) {
		return NULL;
	}
	snake->head->x = x;
	snake->head->y = y;
	snake->head->next = NULL;
	snake->head->prev = NULL;
	snake->tail = snake->head;
	snake->length = 0;
	snake->direction = SNAKE_UP;
	snake->color = GREEN;

    if (!is_random_seed_shared) {
        rng_seed(TIM2->CNT);
    }

	return snake;
}

snake_status_t snake_enqueue(snake_t *snake, uint8_t x, uint8_t y) {
	snake_part_t *part = (snake_part_t*) pvPortMalloc(sizeof(snake_part_t));
	if (part == NULL) {
		return SNAKE_MALLOC_FAILED;
	}
	part->x = x;
	part->y = y;
	part->next = snake->head;
	part->prev = NULL;
	snake->head->prev = part;
	snake->head = part;
	snake->length++;

	return SNAKE_OK;
}

snake_status_t snake_dequeue(snake_t *snake) {
	snake_part_t *part = snake->tail;
	part->prev->next = NULL;
	snake->tail = part->prev;
	vPortFree(part);
	snake->length--;

	return SNAKE_OK;
}

snake_status_t snake_destroy(snake_t *snake) {
	snake_part_t *part = snake->head;
	while (part != NULL) {
		snake_part_t *next = part->next;
		vPortFree(part);
		part = next;
	}
	vPortFree(snake);

	return SNAKE_OK;
}

uint8_t snake_contains(snake_t *snake, uint8_t x, uint8_t y) {
	if (snake != NULL) {
		snake_part_t *part = snake->head;
		while (part != NULL) {
			if (part->x == x && part->y == y) {
				return 1;
			}
			part = part->next;
		}
	}
	return 0;
}

uint8_t poison_food_contains(snake_field_t *field, uint8_t x, uint8_t y) {
	if (field->poison_food != NULL) {
		snake_poison_food_t *node = field->poison_food;
		while (node != NULL) {
			if (node->x == x && node->y == y && node->duration > 0) {
				return 1;
			}
			node = node->next;
		}
	}
	return 0;
}

uint8_t wall_contains(snake_field_t *field, uint8_t x, uint8_t y) {
	if (field->wall != NULL) {
		snake_wall_t *node = field->wall;
		while (node != NULL) {
			if (node->x == x && node->y == y) {
				return 1;
			}
			node = node->next;
		}
	}
	return 0;
}

snake_collision_t check_collision(snake_field_t *field, uint8_t x, uint8_t y) {

	if (wall_contains(field, x, y)) {
		return WALL_COLLISION;
	}
	if (snake_contains(field->snake1, x, y) || snake_contains(field->snake2, x, y)) {
		return SNAKE_BODY_COLLISION;
	}
	if (field->food.x == x && field->food.y == y) {
		return FOOD_COLLISION;
	}
	if (poison_food_contains(field, x, y)) {
		return POISON_FOOD_COLLISION;
	}

	return 0;
}

snake_wall_t *add_wall_node(snake_wall_t *node, uint8_t x, uint8_t y) {
	snake_wall_t *new_node = (snake_wall_t*) pvPortMalloc(sizeof(snake_wall_t));
	if (new_node == NULL) {
		return NULL;
	}
	new_node->x = x;
	new_node->y = y;
	new_node->visible = 1;
	new_node->next = node;
	new_node->prev = NULL;
	node->prev = new_node;
	return new_node;
}

snake_status_t generate_wall(snake_field_t *field, game_options_t *game_options) {

	snake_wall_t *node;

	if (field->wall != NULL || game_options->difficulty == EASY) {
		return SNAKE_WALL_NOT_GENERATED;
	}

	// Add Top/Bottom Walls

	if (game_options->difficulty >= MEDIUM) {
		for (uint8_t x = 0; x < field->width; x++) {
			node = add_wall_node(field->wall, x, 0);
			if (node == NULL) {
				return SNAKE_MALLOC_FAILED;
			}
			field->wall = node;
			node = add_wall_node(field->wall, x, field->height - 1);
			if (node == NULL) {
				return SNAKE_MALLOC_FAILED;
			}
			field->wall = node;
		}
	}
	// Add Left/Right Walls
	if (game_options->difficulty == MEDIUM) {
		for (uint8_t y = 0; y < field->height; y++) {
			node = add_wall_node(field->wall, 0, y);
			if (node == NULL) {
				return SNAKE_MALLOC_FAILED;
			}
			field->wall = node;
			node = add_wall_node(field->wall, field->width - 1, y);
			if (node == NULL) {
				return SNAKE_MALLOC_FAILED;
			}
			field->wall = node;
		}
	} else {
		uint8_t segment_height = field->height >> 2;
		uint8_t segment_width = field->width >> 2;

		for (uint8_t y = 1; y < segment_height; y++) {
			for (uint8_t x = 0; x < segment_width; x++) {
				node = add_wall_node(field->wall, x, y);
				if (node == NULL) {
					return SNAKE_MALLOC_FAILED;
				}
				field->wall = node;
				node = add_wall_node(field->wall, field->width - 1 - x, y);
				if (node == NULL) {
					return SNAKE_MALLOC_FAILED;
				}
				field->wall = node;
				node = add_wall_node(field->wall, x, field->height - 1 - y);
				if (node == NULL) {
					return SNAKE_MALLOC_FAILED;
				}
				field->wall = node;
				node = add_wall_node(field->wall, field->width - 1 - x,	field->height - 1 - y);
				if (node == NULL) {
					return SNAKE_MALLOC_FAILED;
				}
				field->wall = node;
			}
		}
		for (uint8_t y = segment_height; y < field->height - segment_height; y++) {
			node = add_wall_node(field->wall, 0, y);
			if (node == NULL) {
				return SNAKE_MALLOC_FAILED;
			}
			field->wall = node;
			node = add_wall_node(field->wall, field->width - 1, y);
			if (node == NULL) {
				return SNAKE_MALLOC_FAILED;
			}
			field->wall = node;
		}
	}

	return SNAKE_WALL_GENERATED;
}

snake_status_t destroy_wall(snake_field_t *field) {
	snake_wall_t *node = field->wall;
	while (node != NULL) {
		snake_wall_t *next = node->next;
		vPortFree(node);
		node = next;
	}
	field->wall = NULL;
	return SNAKE_OK;
}

uint8_t is_front_snake_head(snake_field_t *field, uint8_t range, uint8_t x, uint8_t y) {

	snake_t *the_snake;

	for (uint8_t i=0; i<2; i++) {
		if (i==0) {
			the_snake = field->snake1;
		} else {
			the_snake = field->snake2;
		}
		if (the_snake != NULL) {
			snake_part_t *head = the_snake->head;
			switch(the_snake->direction) {
				case SNAKE_UP:
					if(head->y > y && head->y <= y-range && head->x == x)
						return 1;
				case SNAKE_DOWN:
					if (head->y < y && head->y >= y + range && head->x == x)
					return 1;
				case SNAKE_LEFT:
					if (head->x > x && head->x <= x - range && head->y == y)
						return 1;
				case SNAKE_RIGHT:
				if (head->x < x && head->x >= x + range && head->y == y)
					return 1;
				default:
					return 0;
			}
		}
	}
	return 0;
}

snake_status_t spawn_poison_food(snake_field_t *field) {

	if (field->num_poisons_spawned < field->max_poisons && field->poison_spawn_cooldown == 0) {
		field->num_poisons_spawned++;
	} else {
		return SNAKE_OK;
	}

	snake_poison_food_t *node = (snake_poison_food_t*) pvPortMalloc(sizeof(snake_poison_food_t));
	if (node == NULL) {
			return SNAKE_MALLOC_FAILED;
	}

	node->next = field->poison_food;
	node->prev = NULL;

	uint8_t x, y;

	do {
		x = rng_get(&htim5, &htim1,field->width);
		y = rng_get(&htim5, &htim1, field->height);
	} while (check_collision(field, x, y) || is_front_snake_head(field, 2, x, y));

	node->x = x;
	node->y = y;
	node->duration = rng_get(&htim5, &htim1, 30) + 40;

	if (field->poison_food != NULL) {
		field->poison_food->prev = node;
	}
	field->poison_food = node;
	field->poison_spawn_cooldown = (uint8_t)rng_get(&htim5, &htim1, field->poison_cooldown_base) + 5;

	return SNAKE_OK;
}

snake_status_t poison_food_destroy(snake_field_t *field) {
	snake_poison_food_t *part = field->poison_food;
	while (part != NULL) {
		snake_poison_food_t *next = part->next;
		vPortFree(part);
		part = next;
	}
	field->poison_food = NULL;
	field->num_poisons_spawned = 0;

	return SNAKE_OK;
}

snake_status_t poison_food_fade(snake_field_t *field) {
	snake_poison_food_t *node = field->poison_food;
	while (node != NULL) {
		if (node->duration > 0) {
			node->duration--;
			if (node->duration == 0) {
				if (node->prev != NULL) {
					node->prev->next = node->next;
					node->next->prev = node->prev;
				}
				else {
					field->poison_food = node->next;
					field->poison_food->prev = NULL;
				}
				vPortFree(node);
				field->num_poisons_spawned--;
			}
		}
		node = node->next;
	}
	return SNAKE_OK;
}


snake_status_t spawn_food(snake_field_t *field) {
	uint8_t x, y;
	do {
		x = rng_get(&htim5, &htim1, field->width);
		y = rng_get(&htim5, &htim1, field->height);
	} while (check_collision(field, x, y) != NO_COLLISION);
	field->food.x = x;
	field->food.y = y;

	return SNAKE_OK;
}

snake_status_t snake_move(snake_t *snake, snake_field_t *field) {

	snake_part_t *head = snake->head;
	uint8_t x = head->x;
	uint8_t y = head->y;
	switch (snake->direction) {
	case SNAKE_UP:
		if (y == 0) {
			y = field->height - 1;
		} else {
			y = (y - 1);
		}
		break;
	case SNAKE_DOWN:
		if (y ==field->height - 1) {
			y = 0;
		} else {
			y = (y + 1);
		}
		break;
	case SNAKE_LEFT:
		if (x == 0) {
			x = field->width - 1;
		} else {
			x = (x - 1);
		}
		break;
	case SNAKE_RIGHT:
		if (x == field->width - 1) {
			x = 0;
		} else {
			x = (x + 1);
		}
		break;
	case SNAKE_NO_CHANGE:
		break;
	}

	snake_collision_t collision = check_collision(field, x, y);

	if (collision == WALL_COLLISION) {
		return SNAKE_DEATH_BY_WALL;
	}
	if (collision == SNAKE_BODY_COLLISION) {
		return SNAKE_DEATH_BY_SELF;
	}
	if (collision == POISON_FOOD_COLLISION) {
		return SNAKE_DEATH_BY_POISON;
	}
	snake_enqueue(snake, x, y);
	if (x == field->food.x && y == field->food.y) {
		return SNAKE_FOOD_SPAWNED;
	} else {
		snake_dequeue(snake);
	}

	return SNAKE_OK;
}

snake_direction_t snake_get_direction(snes_controller_t *snes_controller, snake_direction_t current_direction) {
	if (snes_controller->current_button_state == SNES_UP_MASK && current_direction != SNAKE_DOWN) {
		return SNAKE_UP;
    } else if (snes_controller->current_button_state == SNES_DOWN_MASK && current_direction != SNAKE_UP) {
		return SNAKE_DOWN;
	} else if (snes_controller->current_button_state == SNES_LEFT_MASK && current_direction != SNAKE_RIGHT) {
		return SNAKE_LEFT;
    } else if (snes_controller->current_button_state == SNES_RIGHT_MASK && current_direction != SNAKE_LEFT) {
		return SNAKE_RIGHT;
    }
	return SNAKE_NO_CHANGE;
}
