/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "splash.h"
#include "snake.h"
#include "menu.h"
#include "ws2812.h"
#include "led_grid.h"
#include "snes_controller.h"
#include "ring_buffer.h"
#include "ui.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct controller_direction_t {
    snake_direction_t current_direction;
    snake_direction_t previous_direction;
} controller_direction_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// ring buffer size
#define RING_BUFFER_SIZE 16
#define MAX_GAME_PACE 60
#define GAME_PACE_STEP 5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim13;
DMA_HandleTypeDef hdma_tim3_ch1_trig;
DMA_HandleTypeDef hdma_tim3_ch3;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for heartbeatTask */
osThreadId_t heartbeatTaskHandle;
const osThreadAttr_t heartbeatTask_attributes = {
  .name = "heartbeatTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for stripShowMutex */
osMutexId_t stripShowMutexHandle;
const osMutexAttr_t stripShowMutex_attributes = {
  .name = "stripShowMutex"
};
/* USER CODE BEGIN PV */

uint16_t **grid_lookup;
uint8_t *brightness_lookup = NULL;
led_t led;

snes_controller_t controller1;
snes_controller_t controller2;
RingBuffer controller1_buffer;
RingBuffer controller2_buffer;

uint8_t update_screen_flag = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM5_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM13_Init(void);
void StartDefaultTask(void *argument);
void StartHeartbeatTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_USART2_UART_Init();
  MX_TIM13_Init();
  /* USER CODE BEGIN 2 */
    snes_controller_init(&controller1, &htim5, SNES_LATCH_GPIO_Port,
    SNES_LATCH_Pin,
    SNES_CLOCK_GPIO_Port, SNES_CLOCK_Pin, SNES_DATA0_GPIO_Port,
    SNES_DATA0_Pin);
    snes_controller_init(&controller2, &htim5, SNES_LATCH_GPIO_Port,
    SNES_LATCH_Pin,
    SNES_CLOCK_GPIO_Port, SNES_CLOCK_Pin, SNES_DATA1_GPIO_Port,
    SNES_DATA1_Pin);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of stripShowMutex */
  stripShowMutexHandle = osMutexNew(&stripShowMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_Base_Start(&htim5);
    HAL_TIM_Base_Start_IT(&htim13);

  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of heartbeatTask */
  heartbeatTaskHandle = osThreadNew(StartHeartbeatTask, NULL, &heartbeatTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 168-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 105-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 84-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief TIM13 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM13_Init(void)
{

  /* USER CODE BEGIN TIM13_Init 0 */

  /* USER CODE END TIM13_Init 0 */

  /* USER CODE BEGIN TIM13_Init 1 */

  /* USER CODE END TIM13_Init 1 */
  htim13.Instance = TIM13;
  htim13.Init.Prescaler = 8400-1;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim13.Init.Period = 410-1;
  htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM13_Init 2 */

  /* USER CODE END TIM13_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  /* DMA1_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_SNES1_Pin|LED_SNES0_Pin|LED_HB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OLED_RST_Pin|USB_OTG_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OLED_CS_Pin|OLED_DC_Pin|HID_OUT_Pin|SNES_LATCH_Pin
                          |EEPROM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SNES_CLOCK_GPIO_Port, SNES_CLOCK_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : PC13 PC0 PC1 PC4
                           PC5 PC6 PC9 PC10
                           PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PH0 PH1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_SNES1_Pin LED_SNES0_Pin LED_HB_Pin */
  GPIO_InitStruct.Pin = LED_SNES1_Pin|LED_SNES0_Pin|LED_HB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SNES_DATA1_Pin SNES_DATA0_Pin */
  GPIO_InitStruct.Pin = SNES_DATA1_Pin|SNES_DATA0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 PA9
                           PA11 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_9
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OLED_RST_Pin */
  GPIO_InitStruct.Pin = OLED_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OLED_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OLED_CS_Pin OLED_DC_Pin HID_OUT_Pin EEPROM_Pin */
  GPIO_InitStruct.Pin = OLED_CS_Pin|OLED_DC_Pin|HID_OUT_Pin|EEPROM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB15 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SNES_LATCH_Pin SNES_CLOCK_Pin */
  GPIO_InitStruct.Pin = SNES_LATCH_Pin|SNES_CLOCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OTG_EN_Pin */
  GPIO_InitStruct.Pin = USB_OTG_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_OTG_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OTG_OC_Pin */
  GPIO_InitStruct.Pin = USB_OTG_OC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OTG_OC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
    HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_1);
    led.data_sent_flag = 1;
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */

    // Get controller states
    snes_controller_read2(&controller1, &controller2);

    // Show Game Splash Screen
    splash();

    /* Local Game variables */
    uint8_t game_over = 0;
    uint8_t game_reset = 0;
    uint8_t game_in_progress = 0;
    uint8_t game_pause = 0;
    uint16_t game_score[2] = { 0, 0 };
    uint16_t delay_counter = 0;
    uint8_t game_pace = 10;
    uint16_t best_score = 3;
    uint8_t game_level = 1;
    uint16_t apples_eaten[2] = { 0, 0 };
    uint8_t update_screen = 0;
    uint8_t display_contrast = 20;
    uint8_t level_up_interval = 5;
    uint8_t level_up_counter = 0;
    uint8_t order_of_play = 0;
    uint8_t perf_counter = 0;
    uint8_t death = 0;

    snake_status_t status = SNAKE_OK;
    snake_status_t status2 = SNAKE_OK;
    snake_status_t death_reason = SNAKE_DEATH_BY_WALL;
    snake_field_t *field;
//	snake_food_t food = { 0, 0 };
    snake_direction_t dir = SNAKE_NO_CHANGE;
    controller_direction_t dir_buffer;
    game_options_t game_options = { ONE_PLAYER, EASY, NO_POISON, 5 };

    WS2812_error_t led_error = WS2812_OK;
    uint16_t board_size = (GRID_SIZE * GRID_WIDTH) * (GRID_SIZE * GRID_HEIGHT);

    uint32_t start_time;
    uint32_t end_time;
    uint32_t elapsed_time;

    uint32_t game_start_time;
    uint32_t game_end_time;
    uint32_t game_elapsed_time = 0;

    uint8_t oled_buffer[20];

    /* Generate and initialize the grid lookup table */
    grid_lookup = generate_lookup_grid(GRID_SIZE * GRID_WIDTH, GRID_SIZE * GRID_HEIGHT, GRID_WIDTH,
    GRID_HEIGHT);

    /* Generate and initialize the brightness lookup table */
    brightness_lookup = generate_brightness_lookup_table(4);

    /* Initialize the LED grid */

    led_error = WS2812_init(&led, &htim3, TIM_CHANNEL_1, htim3.Init.Period, board_size, 0);
    if (led_error != WS2812_OK) {
        Error_Handler();
    }

    led.data_sent_flag = 1;

    WS2812_fill(&led, 0, 0, 32);
//    WS2812_set_brightness(&led, 5);
    WS2812_send(&led);

    osDelay(500);

    WS2812_clear(&led);
//    WS2812_set_brightness(&led, led.brightness);
    WS2812_send(&led);

    osDelay(10);

    // Grid test

    //grid_test(&led, 32, 16);

    // Initialize the ring buffer for controllers
    if (ring_buffer_init(&controller1_buffer, RING_BUFFER_SIZE, sizeof(controller_direction_t))
            == RING_BUFFER_MALLOC_FAILED) {
        Error_Handler();
    }
    if (ring_buffer_init(&controller2_buffer, RING_BUFFER_SIZE, sizeof(controller_direction_t))
            == RING_BUFFER_MALLOC_FAILED) {
        Error_Handler();
    }

    controller1.previous_button_state = 0xffff;
    controller2.previous_button_state = 0xffff;

    field = snake_field_init(GRID_SIZE * GRID_WIDTH, GRID_SIZE * GRID_HEIGHT);
    if (field == NULL) {
        Error_Handler();
    }

    ssd1306_Fill(Black);

    /* Infinite loop */
    for (;;) {
        if (!game_in_progress) {
            snes_controller_read2(&controller1, &controller2);
            if (!game_over) {

                if (controller1.current_button_state != controller1.previous_button_state) {

                    ssd1306_SetCursor(0, 3);
                    ssd1306_WriteString("Controller 1", Font_7x10, White);
                    snprintf((char*) oled_buffer, 16, "c: %04x p: %04x", controller1.current_button_state,
                            controller1.previous_button_state);
                    ssd1306_SetCursor(0, 15);
                    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
                    ssd1306_UpdateScreen();
                }
                if (controller2.current_button_state != controller2.previous_button_state) {
                    ssd1306_SetCursor(0, 32);
                    ssd1306_WriteString("Controller 2", Font_7x10, White);
                    snprintf((char*) oled_buffer, 16, "c: %04x p: %04x", controller2.current_button_state,
                            controller2.previous_button_state);
                    ssd1306_SetCursor(0, 42);
                    ssd1306_WriteString((char*) oled_buffer, Font_7x10, White);
                    ssd1306_UpdateScreen();
                }
            } else if (game_over && !delay_counter) {
                ssd1306_Reset();
                ssd1306_Init();
                ssd1306_Fill(Black);
                ssd1306_SetCursor(14, 2);
                ssd1306_WriteString("GAME OVER", Font_11x18, White);
//                ssd1306_SetCursor(25, 54);
//                ssd1306_WriteString("Press Start", Font_7x10, White);
                delay_counter++;
            }
            if (game_over && delay_counter) {

                ui_game_over_screen(&game_options, game_score, best_score, delay_counter, game_level,
                        death_reason, apples_eaten, game_elapsed_time);
                delay_counter++;
                if (delay_counter > UI_DELAY * 3) {
                    delay_counter = 1;
                }
            }
            if (controller1.current_button_state == SNES_START_MASK) {

                menu_game_options(&game_options, &controller1);
                generate_wall(field, &game_options);
                game_in_progress = 1;

                // Set up common game variables
                game_score[0] = 0;
                game_score[1] = 0;
                delay_counter = 0;
                level_up_interval = 5;
                level_up_counter = 0;
                game_level = 1;
                apples_eaten[0] = 0;
                apples_eaten[1] = 0;
                death = 0;

                field->num_poisons_spawned = 0;
                field->poison_spawn_cooldown = 5;

                uint8_t start_x = field->width >> 1;
                uint8_t start_y = field->height >> 1;

                if (game_options.difficulty == EASY) {
                    field->max_poisons = game_options.poison ? 3 : 0;
                    field->poison_cooldown_base = 20;
                    game_pace = MAX_GAME_PACE;
                } else if (game_options.difficulty == MEDIUM) {
                    field->max_poisons = game_options.poison ? 5 : 0;
                    field->poison_cooldown_base = 15;
                    start_y = field->height - 3;
                    game_pace = MAX_GAME_PACE - GAME_PACE_STEP;
                } else if (game_options.difficulty == HARD) {
                    field->max_poisons = game_options.poison ? 7 : 0;
                    field->poison_cooldown_base = 10;
                    start_y = field->height - 3;
                    game_pace = MAX_GAME_PACE - (GAME_PACE_STEP * 2);
                } else if (game_options.difficulty == INSANE) {
                    field->max_poisons = game_options.poison ? 9 : 0;
                    field->poison_cooldown_base = 5;
                    start_y = field->height - 3;
                    game_pace = MAX_GAME_PACE - (GAME_PACE_STEP * 4);
                }

                // Set Up One Player Game Mode
                if (game_options.num_players == ONE_PLAYER) {
                    field->snake1 = snake_init(start_x, start_y);
                    if (field->snake1 == NULL) {
                        Error_Handler();
                    }
                    field->snake1->direction = SNAKE_UP;

                    status = snake_enqueue(field->snake1, start_x, start_y - 1);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = snake_enqueue(field->snake1, start_x, start_y - 2);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                    status = spawn_food(field);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    // Set Up Two Player Game Mode
                } else if (game_options.num_players == TWO_PLAYERS) {

                    field->snake1 = snake_init(start_x - 4, start_y);
                    if (field->snake1 == NULL) {
                        Error_Handler();
                    }
                    field->snake1->direction = SNAKE_UP;
                    field->snake1->color = GREEN;

                    field->snake2 = snake_init(start_x + 4, start_y);
                    if (field->snake2 == NULL) {
                        Error_Handler();
                    }
                    field->snake2->direction = SNAKE_UP;
                    field->snake2->color = BLUE;

                    status = snake_enqueue(field->snake1, start_x - 4, start_y - 1);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = snake_enqueue(field->snake2, start_x + 4, start_y - 1);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = snake_enqueue(field->snake1, start_x - 4, start_y - 2);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = snake_enqueue(field->snake2, start_x + 4, start_y - 2);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }

                    status = spawn_food(field);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                }

                // Draw OLED screen
                ssd1306_Reset();
                ssd1306_Init();
                ssd1306_Fill(Black);

                if (game_options.num_players == ONE_PLAYER)
                    ui_one_player(game_score[0], best_score, game_level);
                else if (game_options.num_players == TWO_PLAYERS)
                    ui_two_player(game_score[0], game_score[1], game_level);

                game_start_time = __HAL_TIM_GET_COUNTER(&htim2);
            }
            osDelay(10);
        }

        if (game_in_progress) {
            start_time = __HAL_TIM_GET_COUNTER(&htim2);
            snes_controller_read2(&controller1, &controller2);
            if (controller1.current_button_state != controller1.previous_button_state
                    && controller1.current_button_state) {
                if (!game_pause) {
                    dir = snake_get_direction(&controller1, field->snake1->direction);
                    if (dir != SNAKE_NO_CHANGE && !is_ring_buffer_full(&controller1_buffer)) {
                        dir_buffer.current_direction = dir;
                        dir_buffer.previous_direction = field->snake1->direction;
                        ring_buffer_enqueue(&controller1_buffer, (void*) &dir_buffer);
                    }
                }
                if (controller1.current_button_state & SNES_START_MASK) {
                    game_pause = !game_pause;
                    update_screen = 1;
                } else if (controller1.current_button_state & SNES_SELECT_MASK) {
                    game_reset = 1;
                } else if (controller1.current_button_state & SNES_R_MASK) {
                    delay_counter = 0;
                    game_pace -= GAME_PACE_STEP;
                    if (game_pace <= GAME_PACE_STEP) {
                        game_pace = GAME_PACE_STEP;
                    }
                    update_screen = 1;
                } else if (controller1.current_button_state & SNES_L_MASK) {
                    delay_counter = 0;
                    game_pace += GAME_PACE_STEP;
                    if (game_pace >= MAX_GAME_PACE) {
                        game_pace = MAX_GAME_PACE;
                    }
                    update_screen = 1;
                } else if (controller1.current_button_state & SNES_A_MASK) {
                    display_contrast -= 5;
                    if (display_contrast == 0) {
                        display_contrast = 5;
                    }
                    ssd1306_SetContrast(display_contrast);
                } else if (controller1.current_button_state & SNES_X_MASK) {
                    display_contrast += 5;
                    if (display_contrast >= 250) {
                        display_contrast = 255;
                    }
                    ssd1306_SetContrast(display_contrast);
                }
            }
            if (controller2.current_button_state != controller2.previous_button_state
                    && controller2.current_button_state) {
                if (!game_pause && game_options.num_players == TWO_PLAYERS) {
                    dir = snake_get_direction(&controller2, field->snake2->direction);
                    if (dir != SNAKE_NO_CHANGE && !is_ring_buffer_full(&controller2_buffer)) {
                        dir_buffer.current_direction = dir;
                        dir_buffer.previous_direction = field->snake2->direction;
                        ring_buffer_enqueue(&controller2_buffer, (void*) &dir_buffer);
                    }
                }
            }
            if (delay_counter >= game_pace && !game_pause) {
                delay_counter = 0;
                order_of_play = !order_of_play;
                if (!is_ring_buffer_empty(&controller1_buffer)) {
                    ring_buffer_dequeue(&controller1_buffer, &dir_buffer);
                    field->snake1->direction = dir_buffer.current_direction;
                }
                if (!is_ring_buffer_empty(&controller2_buffer) && game_options.num_players == TWO_PLAYERS) {
                    ring_buffer_dequeue(&controller2_buffer, &dir_buffer);
                    field->snake2->direction = dir_buffer.current_direction;
                }

                if (game_options.num_players == TWO_PLAYERS) {
                    if (order_of_play) {
                        status = snake_move(field->snake1, field);
                        status2 = snake_move(field->snake2, field);
                    } else {
                        status2 = snake_move(field->snake2, field);
                        status = snake_move(field->snake1, field);
                    }
                } else {
                    status = snake_move(field->snake1, field);
                }

                if (status == SNAKE_FOOD_SPAWNED || status2 == SNAKE_FOOD_SPAWNED) {
                    spawn_food(field);
                    if (status == SNAKE_FOOD_SPAWNED) {
                        apples_eaten[0]++;
                        game_score[0] += game_options.difficulty + 1 + (game_level >> 1);
                    } else if (status2 == SNAKE_FOOD_SPAWNED) {
                        apples_eaten[1]++;
                        game_score[1] += game_options.difficulty + 1 + (game_level >> 1);
                    }

                    level_up_counter++;

                    if (game_options.num_players == ONE_PLAYER) {
                        if (game_score[0] > best_score) {
                            best_score = game_score[0];
                        }
                    }

                    if (level_up_counter >= level_up_interval) {
                        game_level++;
                        game_pace -= GAME_PACE_STEP;
                        if (game_pace <= GAME_PACE_STEP) {
                            game_pace = GAME_PACE_STEP;
                        }
                        level_up_interval += 5;
                        level_up_counter = 0;
                        if (game_pace == 0) {
                            game_pace = 1;
                        }
                        if (game_level % 2 == 1 && game_options.poison) {
                            field->max_poisons++;
                        }
                    }
                    update_screen = 1;
                }
                if (status == SNAKE_DEATH_BY_SELF || status == SNAKE_DEATH_BY_WALL
                        || status == SNAKE_DEATH_BY_POISON) {
                    death_reason = status;
                    death = 1;
                }
                if ((status2 == SNAKE_DEATH_BY_SELF || status2 == SNAKE_DEATH_BY_WALL
                        || status2 == SNAKE_DEATH_BY_POISON) && game_options.num_players == TWO_PLAYERS) {
                    death_reason = status2;
                    death = 1;
                }

                if (death) {
                    status = poison_food_destroy(field);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                    status = destroy_wall(field);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                    status = snake_destroy(field->snake1);
                    if (status != SNAKE_OK) {
                        Error_Handler();
                    }
                    if (game_options.num_players == TWO_PLAYERS) {
                        status = snake_destroy(field->snake2);
                        if (status != SNAKE_OK) {
                            Error_Handler();
                        }
                    }
                    if (status == SNAKE_OK) {
                        field->snake1 = NULL;
                        field->snake2 = NULL;
                    }
                    game_over = 1;
                    game_in_progress = 0;
                    delay_counter = 0;
                    if (game_score[0] > best_score) {
                        best_score = game_score[0];
                    }
                    game_end_time = __HAL_TIM_GET_COUNTER(&htim2);
                    game_elapsed_time = (game_end_time - game_start_time) / 1000000;
                } else {
                    if (field->poison_spawn_cooldown > 0) {
                        field->poison_spawn_cooldown--;
                    }
                    poison_food_fade(field);
                    spawn_poison_food(field);
                    refresh_grid(&led, field, &game_options);

                }
            } else {
                delay_counter++;
            }
            if (update_screen) {

                if (game_options.num_players == ONE_PLAYER)
                    ui_one_player(game_score[0], best_score, game_level);
                else if (game_options.num_players == TWO_PLAYERS)
                    ui_two_player(game_score[0], game_score[1], game_level);

                if (game_pause) {
                    ssd1306_SetCursor(31, 24);
                    ssd1306_WriteString("PAUSED", Font_11x18, White);
                } else {
                    ssd1306_SetCursor(31, 24);
                    ssd1306_WriteString("       ", Font_11x18, White);
                }
            }

            end_time = __HAL_TIM_GET_COUNTER(&htim2);
            elapsed_time = end_time - start_time;
            game_elapsed_time = (end_time - game_start_time) / 1000000;

            if (!perf_counter || (update_screen && update_screen_flag)) {
                ui_latency_ms(elapsed_time);
                ui_elapsed_time(game_elapsed_time);
                perf_counter = 100;

                ssd1306_UpdateScreen();
                update_screen = 0;
            } else if (update_screen && update_screen_flag) {
                ssd1306_UpdateScreen();
                update_screen = 0;
            }
            perf_counter--;
            osDelay(3);
        }
    }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartHeartbeatTask */
/**
 * @brief Function implementing the heartbeatTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartHeartbeatTask */
void StartHeartbeatTask(void *argument)
{
  /* USER CODE BEGIN StartHeartbeatTask */
    /* Infinite loop */
    for (;;) {
        HAL_GPIO_TogglePin(LED_HB_GPIO_Port, LED_HB_Pin);
        if (!controller1.is_active) {
            HAL_GPIO_WritePin(LED_SNES0_GPIO_Port, LED_SNES0_Pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(LED_SNES0_GPIO_Port, LED_SNES0_Pin, GPIO_PIN_SET);
        }
        if (!controller2.is_active) {
            HAL_GPIO_WritePin(LED_SNES1_GPIO_Port, LED_SNES1_Pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(LED_SNES1_GPIO_Port, LED_SNES1_Pin, GPIO_PIN_SET);
        }
        osDelay(500);
    }
  /* USER CODE END StartHeartbeatTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
    if (htim->Instance == TIM13) {
        update_screen_flag = 1;
    }
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
