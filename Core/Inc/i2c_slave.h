/*
 * i2c_slave.h
 *
 *  Created on: Feb 6, 2024
 *      Author: josh
 */

#ifndef INC_I2C_SLAVE_H_
#define INC_I2C_SLAVE_H_

typedef struct {               // Register Map
    uint16_t score;            // 0x00
    uint16_t high_score;       // 0x02
    uint16_t number_apples;    // 0x04
    uint8_t level;             // 0x06
    uint8_t game_status;       // 0x07
} I2C_Register_t;

#endif /* INC_I2C_SLAVE_H_ */
