/*
 * i2c_slave.c
 *
 *  Created on: March 16, 2024
 *      Author: Joshua Butler, MD, MHI
 */

#include "main.h"
#include "i2c_slave.h"
#include "scoreboard.h"
#include <string.h>
#include "led_indicator.h"

volatile uint8_t i2c_rx_buffer[I2C_BUFFER_SIZE];
volatile uint8_t i2c_rx_index = 0;
volatile uint8_t i2c_tx_index = 0;
volatile uint8_t bytes_received = 0;
volatile uint8_t start_position = 0;
volatile scoreboard_t i2c_register_struct;
volatile uint8_t i2c_register[REGISTERS_SIZE] = { 0 };

extern I2C_HandleTypeDef hi2c2;
extern RTC_HandleTypeDef hrtc;
extern led_indicator_t scoreboard_led;

//const uint8_t register_size[] = { sizeof(i2c_register_struct.console_info),
//        sizeof(i2c_register_struct.current_game_state), sizeof(i2c_register_struct.current_game_state2),
//        sizeof(i2c_register_struct.current_score1), 0, sizeof(i2c_register_struct.current_score2), 0,
//        sizeof(i2c_register_struct.number_apples1), 0, sizeof(i2c_register_struct.number_apples2), 0,
//        sizeof(i2c_register_struct.high_score), 0, sizeof(i2c_register_struct.playing_time), 0,
//        sizeof(i2c_register_struct.num_apples_easy), 0, sizeof(i2c_register_struct.num_apples_medium), 0,
//        sizeof(i2c_register_struct.num_apples_hard), 0, sizeof(i2c_register_struct.num_apples_insane), 0,
//        sizeof(i2c_register_struct.high_score_easy), 0, sizeof(i2c_register_struct.high_score_medium), 0,
//        sizeof(i2c_register_struct.high_score_hard), 0, sizeof(i2c_register_struct.high_score_insane), 0,
//        sizeof(i2c_register_struct.initials_easy), 0, 0, sizeof(i2c_register_struct.initials_medium), 0, 0,
//        sizeof(i2c_register_struct.initials_hard), 0, 0, sizeof(i2c_register_struct.initials_insane), 0, 0,
//        sizeof(i2c_register_struct.date_time), 0, 0, 0, 0 };

void volatile_memcpy(volatile void *dest, void *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        *((volatile uint8_t*) dest + i) = *((uint8_t*) src + i);
    }
}

void volatile_memset(volatile void *dest, uint8_t value, size_t n) {
    for (size_t i = 0; i < n; i++) {
        *((volatile uint8_t*) dest + i) = value;
    }
}

void initialize_register() {
    i2c_register_struct.console_info = 0;
    i2c_register_struct.current_game_state = 0;
    i2c_register_struct.current_game_state2 = 0;
    i2c_register_struct.current_game_state3 = 0;
    i2c_register_struct.current_score1 = 0;
    i2c_register_struct.current_score2 = 0;
    i2c_register_struct.number_apples1 = 0;
    i2c_register_struct.number_apples2 = 0;
    i2c_register_struct.high_score = 0;
    i2c_register_struct.playing_time = 0;
    i2c_register_struct.num_apples_easy = 0;
    i2c_register_struct.num_apples_medium = 0;
    i2c_register_struct.num_apples_hard = 0;
    i2c_register_struct.num_apples_insane = 0;
    i2c_register_struct.high_score_easy = 0;
    i2c_register_struct.high_score_medium = 0;
    i2c_register_struct.high_score_hard = 0;
    i2c_register_struct.high_score_insane = 0;
    volatile_memset(i2c_register_struct.initials_easy, 0, 3);
    volatile_memset(i2c_register_struct.initials_medium, 0, 3);
    volatile_memset(i2c_register_struct.initials_hard, 0, 3);
    volatile_memset(i2c_register_struct.initials_insane, 0, 3);
    i2c_register_struct.date_time = 0;
    volatile_memset(i2c_register, 0, REGISTERS_SIZE);
}

void struct2register() {
    uint8_t i = 0;
    i2c_register[i++] = i2c_register_struct.console_info;
    i2c_register[i++] = i2c_register_struct.current_game_state;
    i2c_register[i++] = i2c_register_struct.current_game_state2;
    i2c_register[i++] = i2c_register_struct.current_game_state3;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.current_score1 >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.current_score1 & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.current_score2 >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.current_score2 & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.number_apples1 >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.number_apples1 & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.number_apples2 >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.number_apples2 & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.high_score >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.high_score & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.playing_time >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.playing_time & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.num_apples_easy >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.num_apples_easy & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.num_apples_medium >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.num_apples_medium & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.num_apples_hard >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.num_apples_hard & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.num_apples_insane >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.num_apples_insane & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.high_score_easy >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.high_score_easy & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.high_score_medium >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.high_score_medium & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.high_score_hard >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.high_score_hard & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.high_score_insane >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.high_score_insane & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_easy[0];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_easy[1];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_easy[2];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_medium[0];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_medium[1];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_medium[2];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_hard[0];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_hard[1];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_hard[2];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_insane[0];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_insane[1];
    i2c_register[i++] = (uint8_t) i2c_register_struct.initials_insane[2];
    i2c_register[i++] = (uint8_t) (i2c_register_struct.date_time >> 24) & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.date_time >> 16) & 0xFF;
    i2c_register[i++] = (uint8_t) (i2c_register_struct.date_time >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) i2c_register_struct.date_time & 0xFF;
}

void update_register(game_stats_t game_stats[], uint16_t current_score[], uint16_t best_score,
        uint16_t number_apples[], uint8_t level, uint8_t game_in_progress, uint8_t game_pause,
        uint8_t game_over, uint8_t game_pace, uint8_t clock_sync_flag, uint32_t game_elapsed_time,
        game_options_t *game_options, grid_size_options_t grid_size_options) {

    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    uint8_t console_id = ((I2C2->OAR1 & 0x7E) >> 1) - 15;
    i2c_register_struct.console_info = console_id | CONSOLE_SIGNATURE
            | (clock_sync_flag << CONSOLE_CLOCK_SHIFT)
            | (((uint8_t) game_options->difficulty & 0x0F) << GAME_LEVEL_MODE_SHIFT);

    uint8_t game_progress = 0;
    if (game_in_progress) {
        game_progress = 1;
    } else if (game_pause) {
        game_progress = 2;
    } else if (game_over) {
        game_progress = 3;
    }

    i2c_register_struct.current_game_state = (game_progress & 0x03) | (level << GAME_PLAYING_LEVEL_SHIFT);
    i2c_register_struct.current_game_state2 = (uint8_t) game_options->num_players
            | (game_pace << GAME_SPEED_SHIFT) | ((uint8_t) game_options->poison << GAME_POISON_SHIFT);
    i2c_register_struct.current_game_state3 = (game_over & GAME_CAUSE_OF_DEATH)
            | ((grid_size_options << GAME_GRID_SIZE_SHIFT) & GAME_GRID_SIZE);
    i2c_register_struct.current_score1 = current_score[0];
    i2c_register_struct.current_score2 = current_score[1];
    i2c_register_struct.number_apples1 = number_apples[0];
    i2c_register_struct.number_apples2 = number_apples[1];
    i2c_register_struct.high_score = best_score;
    i2c_register_struct.playing_time = (uint16_t) game_elapsed_time;
    i2c_register_struct.num_apples_easy = game_stats[EASY].num_apples_eaten;
    i2c_register_struct.num_apples_medium = game_stats[MEDIUM].num_apples_eaten;
    i2c_register_struct.num_apples_hard = game_stats[HARD].num_apples_eaten;
    i2c_register_struct.num_apples_insane = game_stats[INSANE].num_apples_eaten;
    i2c_register_struct.high_score_easy = game_stats[EASY].high_score;
    i2c_register_struct.high_score_medium = game_stats[MEDIUM].high_score;
    i2c_register_struct.high_score_hard = game_stats[HARD].high_score;
    i2c_register_struct.high_score_insane = game_stats[INSANE].high_score;
    volatile_memcpy(i2c_register_struct.initials_easy, game_stats[EASY].player_name, 3);
    volatile_memcpy(i2c_register_struct.initials_medium, game_stats[MEDIUM].player_name, 3);
    volatile_memcpy(i2c_register_struct.initials_hard, game_stats[HARD].player_name, 3);
    volatile_memcpy(i2c_register_struct.initials_insane, game_stats[INSANE].player_name, 3);

    i2c_register_struct.date_time = (sDate.Year << YEAR_SHIFT) | (sDate.Month << MONTH_SHIFT)
            | (sDate.Date << DAY_SHIFT) | (sTime.Hours << HOUR_SHIFT) | (sTime.Minutes << MINUTE_SHIFT)
            | (sTime.Seconds << SECOND_SHIFT);
    struct2register();
}

void process_data() {
    // TODO: Implement this function
    if (bytes_received > 0) {
        if (i2c_rx_buffer[0] == 0x05) {
            i2c_register_struct.current_score2 = (i2c_rx_buffer[1] << 8) | i2c_rx_buffer[2];
        }
        bytes_received = 0;
        i2c_rx_index = 0;
        i2c_tx_index = 0;
        struct2register();
    }
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
    if (TransferDirection == I2C_DIRECTION_RECEIVE) { // Master receiving from slave
        start_position = i2c_rx_buffer[0];
        i2c_rx_buffer[0] = 0;
        i2c_tx_index = 0;
        led_indicator_set_blink(&scoreboard_led, 40, 6);
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t*) &i2c_register + start_position, 1, I2C_FIRST_FRAME);
    } else if (TransferDirection == I2C_DIRECTION_TRANSMIT) { // Master transmitting to slave
        i2c_rx_index = 0;
        volatile_memset((uint8_t*) i2c_rx_buffer, 0, I2C_BUFFER_SIZE);
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*) &i2c_rx_buffer, 1, I2C_FIRST_FRAME);
    } else {
        Error_Handler();
    }
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_rx_index++;
    bytes_received++;
    if (i2c_rx_index < I2C_BUFFER_SIZE) {
        if (i2c_rx_index == I2C_BUFFER_SIZE - 1) {
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*) &i2c_rx_buffer + i2c_rx_index, 1, I2C_LAST_FRAME);
        } else {
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*) &i2c_rx_buffer + i2c_rx_index, 1, I2C_NEXT_FRAME);
        }
    }
    if (i2c_rx_index == I2C_BUFFER_SIZE) {
        process_data();
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_tx_index++;
    HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t*) &i2c_register + start_position + i2c_tx_index, 1,
            I2C_NEXT_FRAME);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    uint32_t error = HAL_I2C_GetError(hi2c);
    if (error == HAL_I2C_ERROR_AF) {
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);
        if (i2c_tx_index == 0) { // error occured while slave is receiving
            i2c_rx_index = 0;
            process_data();
        } else { // error occured while slave is transmitting
            i2c_tx_index--;
        }
    } else if (error == HAL_I2C_ERROR_BERR) {
        HAL_I2C_DeInit(hi2c);
        HAL_I2C_Init(hi2c);
        volatile_memset((uint8_t*) i2c_rx_buffer, 0, I2C_BUFFER_SIZE);
        bytes_received = 0;
        i2c_rx_index = 0;
        i2c_tx_index = 0;
    }
    HAL_I2C_EnableListen_IT(hi2c);
}
