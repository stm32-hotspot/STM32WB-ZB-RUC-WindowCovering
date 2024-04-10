/**
 ******************************************************************************
 * @file    AMS_PWM.h
 * @author  MCD Application Team
 * @brief   PWM Header for Arduino Motor Shield-Rev3 driver
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
 *
 ******************************************************************************
 */
#ifndef AMS_PWM_H
#define AMS_PWM_H

#include "AMS_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initializes PWM1 for AMS
 * @param  duty_cycle : Value should be between 0 and 100, 100 means 100% of 3.3V
 * @retval State
 */
bool ams_pwm_Init(uint32_t duty_cycle);

/**
 * @brief  Change PWM Duty cycle
 * @param  duty_cycle : Value should be between 0 and 100, 100 means 100% of 3.3V
 * @retval None
 */
void ams_pwm_change_duty_cycle(uint32_t duty_cycle);

/**
 * @brief  PWM TIM1 IRQ Handler
 * @retval None
 */
void AMS_TIMx_CC_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* AMS_PWM_H */
