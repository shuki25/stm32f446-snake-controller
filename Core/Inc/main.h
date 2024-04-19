/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern const unsigned char snake_game_logo[];
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_SNES1_Pin GPIO_PIN_2
#define LED_SNES1_GPIO_Port GPIOC
#define SNES_DATA1_Pin GPIO_PIN_3
#define SNES_DATA1_GPIO_Port GPIOC
#define LED_GRID0_Pin GPIO_PIN_6
#define LED_GRID0_GPIO_Port GPIOA
#define OLED_RST_Pin GPIO_PIN_5
#define OLED_RST_GPIO_Port GPIOC
#define LED_GRID1_Pin GPIO_PIN_0
#define LED_GRID1_GPIO_Port GPIOB
#define OLED_CS_Pin GPIO_PIN_1
#define OLED_CS_GPIO_Port GPIOB
#define OLED_DC_Pin GPIO_PIN_2
#define OLED_DC_GPIO_Port GPIOB
#define I2C_LED_Pin GPIO_PIN_12
#define I2C_LED_GPIO_Port GPIOB
#define SNES_LATCH_Pin GPIO_PIN_13
#define SNES_LATCH_GPIO_Port GPIOB
#define SNES_CLOCK_Pin GPIO_PIN_14
#define SNES_CLOCK_GPIO_Port GPIOB
#define SB_EN_Pin GPIO_PIN_15
#define SB_EN_GPIO_Port GPIOB
#define SB_LINK_Pin GPIO_PIN_6
#define SB_LINK_GPIO_Port GPIOC
#define SB_LINK_EXTI_IRQn EXTI9_5_IRQn
#define SNES_DATA0_Pin GPIO_PIN_7
#define SNES_DATA0_GPIO_Port GPIOC
#define LED_SNES0_Pin GPIO_PIN_8
#define LED_SNES0_GPIO_Port GPIOC
#define USB_OTG_EN_Pin GPIO_PIN_8
#define USB_OTG_EN_GPIO_Port GPIOA
#define USB_OTG_OC_Pin GPIO_PIN_10
#define USB_OTG_OC_GPIO_Port GPIOA
#define USB_OTG_OC_EXTI_IRQn EXTI15_10_IRQn
#define EEPROM_Pin GPIO_PIN_4
#define EEPROM_GPIO_Port GPIOB
#define LED_HB_Pin GPIO_PIN_5
#define LED_HB_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define VERSION_MAJOR 1
#define VERSION_MINOR 1
#define VERSION_PATCH 1
#define BOARD_REVISION "c"

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
