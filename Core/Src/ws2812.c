/*
 * ws2812.c
 *
 *  Created on: Jan 4, 2024
 *      Author: Joshua Butler, MD, MHI
 *
 *      This file contains the functions to control the WS2812B LED strip.
 */

#include "ws2812.h"
#include <math.h>
#include <stdlib.h>
#include "freertos.h"

const uint8_t color_groups[8][3] = { { 128, 0, 0 }, // red
        { 0, 48, 0 }, // green
        { 0, 0, 64 },  // blue
        { 150, 32, 0 }, // yellow
        { 64, 0, 64 }, // magenta
        { 0, 128, 128 }, // cyan
        { 16, 16, 16 }, // white
        { 0, 0, 0 } // off
};

uint8_t* generate_brightness_lookup_table(uint8_t brightness) {
    uint8_t *table = pvPortMalloc(256 * sizeof(uint8_t));
    if (table == NULL) {
        return NULL;
    }
    if (brightness > 45)
        brightness = 45;
    for (int i = 0; i < 256; i++) {
        float angle = 90 - brightness;
        angle = angle * PI / 180;
        table[i] = (uint8_t) (i / (tan(angle)));
    }
    return table;
}

void destroy_brightness_lookup_table(uint8_t *table) {
    vPortFree(table);
}

WS2812_error_t WS2812_init(led_t *led_obj, TIM_HandleTypeDef *htim, const uint32_t channel,
        const uint8_t counter_period, const uint16_t num_leds, uint8_t sacrificial_led_flag) {

    led_obj->htim = htim;
    led_obj->channel = channel;
    led_obj->counter_period = counter_period;
    led_obj->duty_high = round(counter_period * 2 / 3);
    led_obj->duty_low = round(counter_period * 1 / 3);
    led_obj->num_leds = num_leds;
    led_obj->data_sent_flag = 0;
    led_obj->sacrificial_led_flag = sacrificial_led_flag;
    led_obj->brightness = 5;

    uint16_t adj_num_leds = num_leds + (sacrificial_led_flag * NUM_SACRIFICIAL_LED);

    led_obj->data = (uint8_t**) pvPortMalloc(adj_num_leds * sizeof(uint8_t*));
    if (led_obj->data == NULL) {
        return WS2812_MALLOC_FAILED;
    }
    for (int i = 0; i < adj_num_leds; i++) {
        led_obj->data[i] = (uint8_t*) pvPortMalloc(4 * sizeof(uint8_t));
        if (led_obj->data[i] == NULL) {
            return WS2812_MALLOC_FAILED;
        }
    }
    for (int i = 0; i < adj_num_leds; i++) {
        led_obj->data[i][0] = i;
        led_obj->data[i][1] = 0;
        led_obj->data[i][2] = 0;
        led_obj->data[i][3] = 0;
    }
//    led_obj->mod = (uint8_t**) pvPortMalloc(adj_num_leds * sizeof(uint8_t*));
//    if (led_obj->mod == NULL) {
//        return WS2812_MALLOC_FAILED;
//    }
//    for (int i = 0; i < adj_num_leds; i++) {
//        led_obj->mod[i] = (uint8_t*) pvPortMalloc(4 * sizeof(uint8_t));
//        if (led_obj->mod[i] == NULL) {
//            return WS2812_MALLOC_FAILED;
//        }
//    }
    led_obj->pwm_data = (uint16_t*) pvPortMalloc((sizeof(uint16_t) * ((adj_num_leds * 24) + 60)));
    if (led_obj->pwm_data == NULL) {
        return WS2812_MALLOC_FAILED;
    }
    return WS2812_OK;
}

WS2812_error_t WS2812_destroy(led_t *led_obj) {
    uint16_t adj_num_leds = led_obj->num_leds + (led_obj->sacrificial_led_flag * NUM_SACRIFICIAL_LED);
    for (int i = 0; i < adj_num_leds; i++) {
        vPortFree(led_obj->data[i]);
    }
    vPortFree(led_obj->data);
    vPortFree(led_obj->pwm_data);
    return WS2812_OK;
}

WS2812_error_t WS2812_set_LED(led_t *led_obj, uint16_t LEDnum, uint8_t Red, uint8_t Green, uint8_t Blue) {
    if (LEDnum >= led_obj->num_leds)
        return WS2812_OUT_OF_RANGE;
    led_obj->data[LEDnum][0] = LEDnum;
    led_obj->data[LEDnum][1] = Green;
    led_obj->data[LEDnum][2] = Red;
    led_obj->data[LEDnum][3] = Blue;

    return WS2812_OK;
}

WS2812_error_t WS2812_set_LED_color(led_t *led_obj, uint16_t LEDnum, uint8_t color) {
    if (LEDnum >= led_obj->num_leds)
        return WS2812_OUT_OF_RANGE;
    led_obj->data[LEDnum][0] = LEDnum;
    led_obj->data[LEDnum][1] = color_groups[color][1];
    led_obj->data[LEDnum][2] = color_groups[color][0];
    led_obj->data[LEDnum][3] = color_groups[color][2];

    return WS2812_OK;
}

//void WS2812_set_brightness(led_t *led_obj, uint8_t brightness) {
//#if USE_BRIGHTNESS
//    if (brightness > 45)
//        brightness = 45;
//    uint16_t num_leds = led_obj->num_leds + (led_obj->sacrificial_led_flag * NUM_SACRIFICIAL_LED);
//    for (int i = 0; i < num_leds; i++) {
//        led_obj->mod[i][0] = led_obj->data[i][0];
//        for (int j = 1; j < 4; j++) {
//            float angle = 90 - brightness;
//            angle = angle * PI / 180;
//            led_obj->mod[i][j] = (led_obj->data[i][j]) / (tan(angle));
//        }
//    }
//#endif
//}

void WS2812_clear(led_t *led_obj) {
    uint8_t offset = (led_obj->sacrificial_led_flag * NUM_SACRIFICIAL_LED);
    for (int i = 0; i < led_obj->num_leds + led_obj->sacrificial_led_flag; i++) {
        WS2812_set_LED(led_obj, offset + i, 0, 0, 0);
    }
}

void WS2812_fill(led_t *led_obj, uint8_t Red, uint8_t Green, uint8_t Blue) {
    uint8_t offset = (led_obj->sacrificial_led_flag * NUM_SACRIFICIAL_LED);
    for (int i = 0; i < led_obj->num_leds + led_obj->sacrificial_led_flag; i++) {
        WS2812_set_LED(led_obj, offset + i, Red, Green, Blue);
    }

}

void WS2812_send(led_t *led_obj) {
    uint32_t indx = 0;
    uint32_t color;

    uint16_t num_leds = led_obj->num_leds + (led_obj->sacrificial_led_flag * NUM_SACRIFICIAL_LED);
    for (int i = 0; i < num_leds; i++) {
#if USE_BRIGHTNESS
        color = ((brightness_lookup[led_obj->data[i][1]] << 16)
                | (brightness_lookup[led_obj->data[i][2]] << 8) | (brightness_lookup[led_obj->data[i][3]]));
//        color = ((led_obj->mod[i][1]<<16) | (led_obj->mod[i][2]<<8) | (led_obj->mod[i][3]));
#else
		color = ((led_obj->data[i][1] << 16) | (led_obj->data[i][2] << 8)
				| (led_obj->data[i][3]));
#endif

        for (int j = 23; j >= 0; j--) {
            if (color & (1 << j)) {
                led_obj->pwm_data[indx] = (uint16_t) led_obj->duty_high; // 2/3 of counter_period
            } else
                led_obj->pwm_data[indx] = (uint16_t) led_obj->duty_low; // 1/3 of counter_period
            indx++;
        }
    }

    for (int i = 0; i < 60; i++) {
        led_obj->pwm_data[indx] = 0;
        indx++;
    }

    while (!led_obj->data_sent_flag) {
    }

    led_obj->data_sent_flag = 0;
    HAL_TIM_PWM_Start_DMA(led_obj->htim, led_obj->channel, (uint32_t*) led_obj->pwm_data, indx);
}
