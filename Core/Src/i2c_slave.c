/*
 * i2c_slave.c
 *
 *  Created on: Feb 6, 2024
 *      Author: josh
 */

#include "main.h"
#include "i2c_slave.h"
#include <string.h>

#define I2C_RX_BUFFER_SIZE 16

volatile uint8_t i2c_rx_buffer[I2C_RX_BUFFER_SIZE];
volatile uint8_t i2c_rx_index = 0;
volatile uint8_t i2c_tx_index = 0;
volatile uint8_t i2c_selected_register = 0;
volatile I2C_Register_t i2c_registers;

const uint8_t register_size[] = { sizeof(i2c_registers.score), 0, sizeof(i2c_registers.high_score), 0,
        sizeof(i2c_registers.number_apples), 0, sizeof(i2c_registers.level), 0, sizeof(i2c_registers.game_status) };

void process_data() {
    // TODO: Implement this function
}

void update_register(const uint16_t score, const uint16_t high_score, const uint16_t number_apples,
        const uint8_t level, const uint8_t game_status) {
    i2c_registers.score = score;
    i2c_registers.high_score = high_score;
    i2c_registers.number_apples = number_apples;
    i2c_registers.level = level;
    i2c_registers.game_status = game_status;
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
    if (TransferDirection == I2C_DIRECTION_RECEIVE) {
        i2c_tx_index = i2c_rx_buffer[0];
        i2c_selected_register = i2c_rx_buffer[0];
        i2c_rx_buffer[0] = 0;
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t*) &i2c_registers + i2c_tx_index, 1, I2C_FIRST_FRAME);
    } else if (TransferDirection == I2C_DIRECTION_TRANSMIT) {
        i2c_rx_index = 0;
        memset((uint8_t*) i2c_rx_buffer, 0, I2C_RX_BUFFER_SIZE);
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*) &i2c_rx_buffer, 1, I2C_FIRST_FRAME);

    } else {
        Error_Handler();
    }
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_rx_index++;
    if (i2c_rx_index < I2C_RX_BUFFER_SIZE) {
        if (i2c_rx_index == I2C_RX_BUFFER_SIZE - 1) {
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*) &i2c_rx_buffer + i2c_rx_index, 1, I2C_LAST_FRAME);
        } else {
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*) &i2c_rx_buffer + i2c_rx_index, 1, I2C_NEXT_FRAME);
            i2c_rx_index = 0;
        }
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_tx_index++;
    if (i2c_tx_index <= register_size[i2c_selected_register]) {
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t*) &i2c_registers + i2c_tx_index, 1, I2C_NEXT_FRAME);
    } else {
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t*) &i2c_rx_buffer + i2c_tx_index, 1, I2C_LAST_FRAME);
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    uint32_t error = HAL_I2C_GetError(hi2c);
    if (error == HAL_I2C_ERROR_AF) {
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);
        if (i2c_tx_index == 0) { // error occured while slave is receiving
            process_data();
        } else { // error occured while slave is transmitting
            i2c_tx_index--;

        }
    }
    HAL_I2C_EnableListen_IT(hi2c);
}
