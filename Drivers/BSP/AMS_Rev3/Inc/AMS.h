/**
 ******************************************************************************
 * @file    AMS.h
 * @author  MCD Application Team
 * @brief   Header for Arduino Motor Shield-Rev3 driver
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
 
#ifndef AMS_H
#define AMS_H

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

/**
 * The Motor Shield documentation : 
 * https://docs.arduino.cc/tutorials/motor-shield-rev3/msr3-controlling-dc-motor
 */

#include "AMS_conf.h"
#include "AMS_PWM.h"
#include "AMS_ADC.h"
  
/**
 * @brief  Initializes AMS
 * @param  duty_cycle : duty cycle for PWM (if you want half of the speed, duty cycle = 50 => 50%)
 * @param  max_adc_treshold: max supported current before shut down (0xFFF = 2A = 3.3V)
 * @retval State
 */
bool ams_init(uint32_t duty_cycle, uint32_t max_adc_treshold);

/**
 * @brief  Initializes Brake and Direction GPIO
 * @param  None
 * @retval None
 */
void ams_brake_dir_gpio_init(void);

/**
 * @brief  Start motor in Up Direction
 * @param  None
 * @retval State
 */
bool ams_start_motor_up(void);

/**
 * @brief  Start motor in Down Direction
 * @param  None
 * @retval State
 */
bool ams_start_motor_down(void);

/**
 * @brief  Stop motor
 * @param  None
 * @retval State
 */
bool ams_stop_motor(void);

/**
 * @brief  Set Brake (Brake GPIO High)
 * @param  None
 * @retval State
 */
void ams_set_brake(void);

/**
 * @brief  Release Brake (Brake GPIO Low)
 * @param  None
 * @retval State
 */
void ams_release_brake(void);

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* AMS_H */
