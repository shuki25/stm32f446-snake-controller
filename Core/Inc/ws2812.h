/*
 * ws2812.h
 *
 *  Created on: Jan 4, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This is a library for the WS2812 LED.  Credit goes to the following sources:
 *
 */

#ifndef INC_WS2812_H_
#define INC_WS2812_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "main.h"

#define USE_BRIGHTNESS 1
#define NUM_SACRIFICIAL_LED 1
#define PI 3.14159265358979323846

typedef enum {
    RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, BLACK
} WS2812_color_t;

typedef enum {
    WS2812_ERROR, WS2812_MALLOC_FAILED, WS2812_OUT_OF_RANGE, WS2812_OK
} WS2812_error_t;

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint8_t counter_period;
    uint8_t duty_high;
    uint8_t duty_low;
    uint16_t num_leds;
    uint16_t **grid_lookup;
    uint8_t data_sent_flag;
    uint8_t sacrificial_led_flag;
    uint8_t brightness;
    uint8_t **data;
//    uint8_t **mod;
    uint16_t *pwm_data;
} led_t;

extern uint8_t *brightness_lookup;

uint8_t* generate_brightness_lookup_table(uint8_t brightness);
void destroy_brightness_lookup_table(uint8_t *table);

WS2812_error_t WS2812_init(led_t *led_obj, TIM_HandleTypeDef *htim, const uint32_t channel,
        const uint8_t counter_period, const uint16_t num_leds, uint8_t sacrificial_led_flag);
WS2812_error_t WS2812_destroy(led_t *led_obj);
WS2812_error_t WS2812_set_LED(led_t *led_obj, uint16_t LEDnum, uint8_t Red, uint8_t Green, uint8_t Blue);
WS2812_error_t WS2812_set_LED_color(led_t *led_obj, uint16_t LEDnum, uint8_t color);
//void WS2812_set_brightness(led_t *led_obj, uint8_t brightness);
void WS2812_clear(led_t *led_obj);
void WS2812_fill(led_t *led_obj, uint8_t Red, uint8_t Green, uint8_t Blue);
void WS2812_send(led_t *led_obj);

#ifdef __cplusplus
}
#endif

#endif /* INC_WS2812_H_ */
