/**
 ******************************************************************************
 * @file    AMS_conf.h
 * @author  MCD Application Team
 * @brief   Header for Arduino Motor Shield config Channel A only
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
 
#ifndef AMS_CONF_H
#define AMS_CONF_H

#include "stm32wbxx_hal.h"
#include <stdbool.h>

#define USE_OF_ADC false                  /* enable/disable ADC */

#define REVERSE_MOTOR_DIR true            /* Swap Motor default rotation */

/**
 * @brief  Pre config STM32WB5MM-DK and STM32WB55RG-Nucleo
 * You can copy paste the define and change it for your board
 */
#define DK

#if defined(DK)
/**
 * @brief  DK pre config
 */
 
/* General GPIO conf*/
#define AMS_RELEASE_BREAK_PIN           GPIO_PIN_15          //Arduino D9 = Break
#define AMS_RELEASE_BREAK_GPIO_Port     GPIOD
  
#define AMS_DIR_PIN                     GPIO_PIN_4           //Arduino D12 = Direction
#define AMS_DIR_GPIO_Port               GPIOB

/**
 * @brief  PWM pre config only work on TIM1
 */
/* PWM conf */
#define AMS_PWM_PIN                     GPIO_PIN_14          //Arduino D3 = PWM
#define AMS_PWM_GPIO_Port               GPIOD
#define AMS_TIM_CHANNELx                TIM_CHANNEL_1
#define AMS_TIMx_CC_IRQn                TIM1_CC_IRQn
#define AMS_TIMx_CC_IRQHandler          TIM1_CC_IRQHandler

/**
 * @brief  ADC pre config only work on ADC1
 * ADC_SUPPORT_2_5_MSPS = 0 
 * USE_HAL_ADC_REGISTER_CALLBACKS = 0 */
#define AMS_ADC_PIN                     GPIO_PIN_3           //Arduino A0 = ADC
#define AMS_ADC_GPIO_Port               GPIOC
#define AMS_DMA1_CHANNELx               DMA1_Channel1
#define AMS_DMA1_Channelx_IRQn          DMA1_Channel1_IRQn
#define AMS_DMA1_CHANNELx_IRQHandler    DMA1_Channel1_IRQHandler
#define AMS_ADC_CHANNEL                 ADC_CHANNEL_4

#elif defined(NUCLEO)
/**
 * @brief  Nucleo pre config
 */
 
/* General GPIO conf*/
#define AMS_RELEASE_BREAK_PIN           GPIO_PIN_9           //Arduino D9 = Break
#define AMS_RELEASE_BREAK_GPIO_Port     GPIOA
  
#define AMS_DIR_PIN                     GPIO_PIN_6           //Arduino D12 = Direction
#define AMS_DIR_GPIO_Port               GPIOA

/**
 * @brief  PWM pre config only work on TIM1
 */
/* PWM conf */
#define AMS_PWM_PIN                     GPIO_PIN_10          //Arduino D3 = PWM
#define AMS_PWM_GPIO_Port               GPIOA
#define AMS_TIMx                        TIM1
#define AMS_TIM_CHANNELx                TIM_CHANNEL_3
#define AMS_TIMx_CC_IRQn                TIM1_CC_IRQn
#define AMS_TIMx_CC_IRQHandler          TIM1_CC_IRQHandler

/**
 * @brief  ADC pre config only work on ADC1
 * ADC_SUPPORT_2_5_MSPS = 0 
 * USE_HAL_ADC_REGISTER_CALLBACKS = 0 */
#define AMS_ADC_PIN                     GPIO_PIN_0           //Arduino A0 = ADC
#define AMS_ADC_GPIO_Port               GPIOC
#define AMS_DMA1_CHANNELx               DMA1_Channel3
#define AMS_DMA1_Channelx_IRQn          DMA1_Channel3_IRQn
#define AMS_DMA1_CHANNELx_IRQHandler    DMA1_Channel3_IRQHandler
#define AMS_ADC_CHANNEL                 ADC_CHANNEL_1

#endif

#define ENABLE_GPIOx_CLK(GPIOx) {if (GPIOx == GPIOA)\
                                   __HAL_RCC_GPIOA_CLK_ENABLE();\
                                 else if (GPIOx == GPIOB)\
                                   __HAL_RCC_GPIOB_CLK_ENABLE();\
                                 else if (GPIOx == GPIOC)\
                                   __HAL_RCC_GPIOC_CLK_ENABLE();\
                                 else if (GPIOx == GPIOD)\
                                   __HAL_RCC_GPIOD_CLK_ENABLE();\
                                }

#endif /* AMS_CONF_H */