/*
* eeprom.c
* Created on: 2023-06-13
* Author: Joshua Butler, MD, MHI
*
* This file contains the functions for controlling the EEPROM.
*/

#include "cmsis_os.h"
#include "eeprom.h"
#include <string.h>

eeprom_status_t eeprom_init(eeprom_t *eeprom, I2C_HandleTypeDef *hi2c, GPIO_TypeDef *wc_port, uint16_t wc_pin)
{
    eeprom->hi2c = hi2c;
    eeprom->wc_port = wc_port;
    eeprom->wc_pin = wc_pin;
    if (HAL_GPIO_ReadPin(eeprom->wc_port, eeprom->wc_pin) == GPIO_PIN_SET)
    {
        eeprom->write_protected = 1;
    }
    else
    {
        eeprom->write_protected = 0;
    }
    return EEPROM_OK;
}

eeprom_status_t eeprom_write_protect(eeprom_t *eeprom, uint8_t state)
{
    if (state == 0)
    {
        HAL_GPIO_WritePin(eeprom->wc_port, eeprom->wc_pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(eeprom->wc_port, eeprom->wc_pin, GPIO_PIN_SET);
    }
    if (HAL_GPIO_ReadPin(eeprom->wc_port, eeprom->wc_pin) == GPIO_PIN_SET)
    {
        eeprom->write_protected = 1;
    }
    else
    {
        eeprom->write_protected = 0;
    }
    return EEPROM_OK;
}

eeprom_status_t eeprom_write(eeprom_t *eeprom, uint16_t page, uint16_t offset, uint8_t *data, uint16_t size)
{
    if (eeprom->write_protected == 1)
    {
        return EEPROM_WRITE_PROTECTED;
    }

    HAL_StatusTypeDef status;
    uint16_t address = page * EEPROM_PAGE_SIZE + offset;

    uint8_t *buffer = (uint8_t *)pvPortMalloc(size + 2);
    if (buffer == NULL)
    {
        return EEPROM_MALLOC_FAILED;
    }
    buffer[0] = (uint8_t)((address & 0xFF00) >> 8);
    buffer[1] = (uint8_t)(address & 0xFF);
    memcpy(buffer + 2, data, size);
    status = HAL_I2C_Master_Transmit(eeprom->hi2c, EEPROM_ADDRESS, buffer, size + 2, 1000);
    if (status == HAL_BUSY)
    {
        return EEPROM_BUSY;
    }
    else if (status == HAL_TIMEOUT)
    {
        return EEPROM_TIMEOUT;
    }
    else if (status != HAL_OK)
    {
        return EEPROM_ERROR;
    }
    vPortFree(buffer);

    while(HAL_I2C_Master_Transmit(eeprom->hi2c, EEPROM_ADDRESS, 0, 0, HAL_MAX_DELAY) != HAL_OK);

    return EEPROM_OK;
}

eeprom_status_t eeprom_read(eeprom_t *eeprom, uint16_t page, uint16_t offset, uint8_t *data, uint16_t size)
{

    HAL_StatusTypeDef status;
    uint16_t address = page * EEPROM_PAGE_SIZE + offset;

    uint8_t address_data[2];
    address_data[0] = (uint8_t)((address & 0xFF00) >> 8);
    address_data[1] = (uint8_t)(address & 0xFF);
    
    status = HAL_I2C_Master_Transmit(eeprom->hi2c, EEPROM_ADDRESS, address_data, 2, 1000);
    if (status == HAL_BUSY)
    {
        return EEPROM_BUSY;
    }
    else if (status == HAL_TIMEOUT)
    {
        return EEPROM_TIMEOUT;
    }
    else if (status != HAL_OK)
    {
        return EEPROM_ERROR;
    }

    status = HAL_I2C_Master_Receive(eeprom->hi2c, EEPROM_ADDRESS, data, size, 1000);
    if (status == HAL_BUSY)
    {
        return EEPROM_BUSY;
    }
    else if (status == HAL_TIMEOUT)
    {
        return EEPROM_TIMEOUT;
    }
    else if (status != HAL_OK)
    {
        return EEPROM_ERROR;
    }

    return EEPROM_OK;
}

void eeprom_generate_signature(eeprom_id_t *signature, uint8_t num_entries) {

    memset(signature, 0, sizeof(eeprom_id_t));
    memcpy(signature->signature, EEPROM_SIGNATURE, 8);
    signature->version = EEPROM_VERSION;
    signature->revision = EEPROM_REVISION;
    signature->num_pages = num_entries;
}

eeprom_status_t eeprom_verify_signature(eeprom_id_t *signature, uint8_t num_entries) {

    eeprom_id_t correct_signature;
    eeprom_generate_signature(&correct_signature, num_entries);

    if (memcmp(&correct_signature, signature, sizeof(eeprom_id_t)) != 0) {
        return EEPROM_SIGNATURE_MISMATCH;
    }
    return EEPROM_OK;
}

eeprom_status_t eeprom_get_signature(eeprom_t *eeprom, eeprom_id_t *signature) {
    eeprom_status_t status;
    uint8_t buffer[EEPROM_PAGE_SIZE];

    status = eeprom_read(eeprom, EEPROM_SIGNATURE_PAGE, EEPROM_SIGNATURE_OFFSET, buffer, sizeof(eeprom_id_t));
    if (status != EEPROM_OK) {
        return status;
    }
    memcpy(signature, buffer, sizeof(eeprom_id_t));
    return EEPROM_OK;
}

eeprom_status_t eeprom_write_signature(eeprom_t *eeprom, eeprom_id_t *signature) {
    eeprom_status_t status;
    uint8_t buffer[EEPROM_PAGE_SIZE];

    memcpy(buffer, signature, sizeof(eeprom_id_t));
    if (eeprom->write_protected == 1) {
        status = eeprom_write_protect(eeprom, 0);
        if (status != EEPROM_OK) {
            return status;
        }
    }
    status = eeprom_write(eeprom, EEPROM_SIGNATURE_PAGE, EEPROM_SIGNATURE_OFFSET, buffer, sizeof(eeprom_id_t));

    if (status != EEPROM_OK) {
        return status;
    }

    if (eeprom->write_protected == 0) {
        eeprom_write_protect(eeprom, 1);
    }

    return EEPROM_OK;
}


