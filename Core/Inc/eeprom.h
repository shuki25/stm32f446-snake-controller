/*
* eeprom.h
* Created on: 2023-06-13
* Author: Joshua Butler, MD, MHI
*
* Header file for eeprom.c
*/

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdint.h>
#include "game_stats.h"

#define EEPROM_ADDRESS 0xA0
#define EEPROM_IDENTIFICATION 0xB0
#define EEPROM_PAGE_SIZE 32
#define EEPROM_PAGE_NUM 256


#define EEPROM_SIGNATURE "SNAKEBIT"
#define EEPROM_VERSION 3
#define EEPROM_REVISION 2
#define EEPROM_SIGNATURE_PAGE 0
#define EEPROM_SIGNATURE_OFFSET 0
#define EEPROM_SETTINGS_PAGE 1
#define EEPROM_SETTINGS_OFFSET 0
#define EEPROM_START_PAGE 2

typedef enum {
    EEPROM_OK = 0,
    EEPROM_ERROR = 1,
    EEPROM_BUSY = 2,
    EEPROM_TIMEOUT = 3,
    EEPROM_MALLOC_FAILED = 4,
    EEPROM_WRITE_PROTECTED = 5,
    EEPROM_SIGNATURE_MISMATCH = 6
} eeprom_status_t;

typedef struct {
    I2C_HandleTypeDef *hi2c;
    GPIO_TypeDef *wc_port;
    uint16_t wc_pin;
    uint8_t write_protected;
} eeprom_t;

typedef struct {
    char signature[9];
    uint8_t version;
    uint8_t revision;
    uint8_t num_pages;
} eeprom_id_t;

eeprom_status_t eeprom_init(eeprom_t *eeprom, I2C_HandleTypeDef *hi2c, GPIO_TypeDef *wc_port, uint16_t wc_pin);
eeprom_status_t eeprom_write_protect(eeprom_t *eeprom, uint8_t state);
eeprom_status_t eeprom_write(eeprom_t *eeprom, uint16_t page, uint16_t offset, uint8_t *data, uint16_t size);
eeprom_status_t eeprom_read(eeprom_t *eeprom, uint16_t page, uint16_t offset, uint8_t *data, uint16_t size);
void eeprom_generate_signature(eeprom_id_t *signature, uint8_t num_pages);
eeprom_status_t eeprom_get_signature(eeprom_t *eeprom, eeprom_id_t *signature);
eeprom_status_t eeprom_write_signature(eeprom_t *eeprom, eeprom_id_t *signature);
eeprom_status_t eeprom_verify_signature(eeprom_id_t *signature, uint8_t num_pages);
eeprom_status_t eeprom_write_settings(eeprom_t *eeprom, saved_settings_t *settings);
eeprom_status_t eeprom_get_settings(eeprom_t *eeprom, saved_settings_t *settings);
#ifdef __cplusplus
}
#endif

#endif /* INC_EEPROM_H_ */
