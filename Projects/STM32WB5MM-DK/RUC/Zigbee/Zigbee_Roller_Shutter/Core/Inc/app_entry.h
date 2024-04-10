/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * File Name          : app_entry.h
  * Description        : App entry configuration file for STM32WPAN Middleware.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2024 STMicroelectronics.
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
#ifndef APP_ENTRY_H
#define APP_ENTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/* Exported functions ---------------------------------------------*/
void MX_APPE_Config( void );
void MX_APPE_Init( void );
void MX_APPE_Process( void );
void Init_Exti( void );
void Init_Smps( void );

void LED_Deinit(void);
void LED_On(void);
void LED_Off(void);
void LED_Set_rgb(uint8_t r, uint8_t g, uint8_t b);
void LED_Toggle(uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_ENTRY_H */
