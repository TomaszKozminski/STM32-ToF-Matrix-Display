/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

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
#define PWM_FILL 1
#define TIM8_PRESCALER 3
#define REPETITIONS 64
#define PWM_PERIOD 5
#define B2_Pin GPIO_PIN_0
#define B2_GPIO_Port GPIOC
#define R2_Pin GPIO_PIN_1
#define R2_GPIO_Port GPIOC
#define B1_Pin GPIO_PIN_2
#define B1_GPIO_Port GPIOC
#define R1_Pin GPIO_PIN_3
#define R1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define I2C_PWREN_Pin GPIO_PIN_6
#define I2C_PWREN_GPIO_Port GPIOA
#define BUTTON_Pin GPIO_PIN_7
#define BUTTON_GPIO_Port GPIOA
#define BUTTON_EXTI_IRQn EXTI9_5_IRQn
#define A_Pin GPIO_PIN_4
#define A_GPIO_Port GPIOC
#define C_Pin GPIO_PIN_5
#define C_GPIO_Port GPIOC
#define I2C_LPN_Pin GPIO_PIN_0
#define I2C_LPN_GPIO_Port GPIOB
#define I2C_INT_Pin GPIO_PIN_1
#define I2C_INT_GPIO_Port GPIOB
#define I2C_INT_EXTI_IRQn EXTI1_IRQn
#define I2C_RST_Pin GPIO_PIN_2
#define I2C_RST_GPIO_Port GPIOB
#define CLK_TIM8_Pin GPIO_PIN_6
#define CLK_TIM8_GPIO_Port GPIOC
#define OE_Pin GPIO_PIN_7
#define OE_GPIO_Port GPIOC
#define G2_Pin GPIO_PIN_8
#define G2_GPIO_Port GPIOC
#define G1_Pin GPIO_PIN_9
#define G1_GPIO_Port GPIOC
#define B_Pin GPIO_PIN_10
#define B_GPIO_Port GPIOC
#define D_Pin GPIO_PIN_11
#define D_GPIO_Port GPIOC
#define LAT_Pin GPIO_PIN_12
#define LAT_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */
#define TX_BUFFER_SIZE 512
#define TOF8X8 8
#define TOF4X4 4
#define DIST_FRAME_BYTES (TOF8X8 * TOF8X8 * sizeof(uint16_t)) // 128 bytes
#define ZONE_FRAME_MAX_BYTES 32
#define COMBINED_FRAME_BYTES (DIST_FRAME_BYTES + ZONE_FRAME_MAX_BYTES) //max 160

#define SHORTDELAYS 5
void delay_us(uint16_t us);
void delay_ticks(uint32_t ticks);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
