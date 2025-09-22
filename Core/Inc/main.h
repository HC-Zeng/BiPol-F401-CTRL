/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOC
#define CTR_FAN_Pin GPIO_PIN_0
#define CTR_FAN_GPIO_Port GPIOC
#define ADC_TEMP_Pin GPIO_PIN_3
#define ADC_TEMP_GPIO_Port GPIOA
#define ADC_C1_Pin GPIO_PIN_4
#define ADC_C1_GPIO_Port GPIOA
#define ADC_C2_Pin GPIO_PIN_5
#define ADC_C2_GPIO_Port GPIOA
#define ADC_C3_Pin GPIO_PIN_6
#define ADC_C3_GPIO_Port GPIOA
#define ADC_C3A7_Pin GPIO_PIN_7
#define ADC_C3A7_GPIO_Port GPIOA
#define AUTO_TRIG_Pin GPIO_PIN_12
#define AUTO_TRIG_GPIO_Port GPIOB
#define AUTO_TRIG_EXTI_IRQn EXTI15_10_IRQn
#define OUT_CH1_Pin GPIO_PIN_13
#define OUT_CH1_GPIO_Port GPIOB
#define OUT_CH2_Pin GPIO_PIN_14
#define OUT_CH2_GPIO_Port GPIOB
#define AMT_TRIG_Pin GPIO_PIN_15
#define AMT_TRIG_GPIO_Port GPIOB
#define AMT_TRIG_EXTI_IRQn EXTI15_10_IRQn
#define PHA_Pin GPIO_PIN_6
#define PHA_GPIO_Port GPIOC
#define PHA_EXTI_IRQn EXTI9_5_IRQn
#define PHB_Pin GPIO_PIN_7
#define PHB_GPIO_Port GPIOC
#define SET_Pin GPIO_PIN_8
#define SET_GPIO_Port GPIOC
#define SET_EXTI_IRQn EXTI9_5_IRQn
#define LED_Pin GPIO_PIN_9
#define LED_GPIO_Port GPIOC
#define RST_Pin GPIO_PIN_8
#define RST_GPIO_Port GPIOA
#define DC_Pin GPIO_PIN_11
#define DC_GPIO_Port GPIOA
#define CS_Pin GPIO_PIN_12
#define CS_GPIO_Port GPIOA
#define SCK_Pin GPIO_PIN_10
#define SCK_GPIO_Port GPIOC
#define SDO_Pin GPIO_PIN_11
#define SDO_GPIO_Port GPIOC
#define SDI_Pin GPIO_PIN_12
#define SDI_GPIO_Port GPIOC
#define CTR_C1_Pin GPIO_PIN_4
#define CTR_C1_GPIO_Port GPIOB
#define CTR_C2_Pin GPIO_PIN_5
#define CTR_C2_GPIO_Port GPIOB
#define CTR_C3_Pin GPIO_PIN_6
#define CTR_C3_GPIO_Port GPIOB
#define CTR_C4_Pin GPIO_PIN_7
#define CTR_C4_GPIO_Port GPIOB
#define CTR_SW_Pin GPIO_PIN_8
#define CTR_SW_GPIO_Port GPIOB
#define PRE_EXIT_Pin GPIO_PIN_9
#define PRE_EXIT_GPIO_Port GPIOB
#define PRE_EXIT_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
