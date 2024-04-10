/**
 ******************************************************************************
 * @file    AMS.c
 * @author  MCD Application Team
 * @brief   Arduino Motor Shield driver
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
#include "AMS.h"

extern TIM_HandleTypeDef htim1;

#if USE_OF_ADC
extern ADC_HandleTypeDef hadc1;
__IO   uint16_t   aADCxConvertedData[64]; /* ADC group regular conversion data (array of data) */
#endif /* USE_OF_ADC */

/**
 * @brief  Initializes AMS
 * @param  duty_cycle : duty cycle for PWM (if you want half of the speed, duty cycle = 50 => 50%)
 * @param  max_adc_treshold: max supported current before shut down (0xFFF = 2A = 3.3V)
 * @retval State
 */
bool ams_init(uint32_t duty_cycle, uint32_t max_adc_treshold)
{
  /* GPIO direction/brake Init*/
  ams_brake_dir_gpio_init();
  
  /* PWM Init*/
  if (ams_pwm_Init(duty_cycle)== false)
    return false;

#if USE_OF_ADC
  /* ADC Init*/
  ams_dma_Init();
  
  if (ams_adc_init(max_adc_treshold) == false)
    return false;

  if ( HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
  {
    return false;
  }
#endif /* USE_OF_ADC */

  return true;
}

/**
 * @brief  Initializes Brake and Direction GPIO
 * @param  None
 * @retval None
 */
void ams_brake_dir_gpio_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  HAL_GPIO_WritePin(AMS_RELEASE_BREAK_GPIO_Port, AMS_RELEASE_BREAK_PIN, GPIO_PIN_RESET);     // Arduino D9  = Brake
  HAL_GPIO_WritePin(AMS_DIR_GPIO_Port, AMS_DIR_PIN, GPIO_PIN_RESET);                         // Arduino D12 = Direction
  
  ENABLE_GPIOx_CLK(AMS_RELEASE_BREAK_GPIO_Port);
  ENABLE_GPIOx_CLK(AMS_DIR_GPIO_Port);
  
  GPIO_InitStruct.Pin = AMS_RELEASE_BREAK_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AMS_RELEASE_BREAK_GPIO_Port, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = AMS_DIR_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AMS_DIR_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief  Start motor in Up Direction
 * @param  None
 * @retval State
 */
bool ams_start_motor_up(void)
{
  HAL_GPIO_WritePin(AMS_DIR_GPIO_Port, AMS_DIR_PIN, GPIO_PIN_SET ^ REVERSE_MOTOR_DIR);         //Arduino D12 = Direction
  ams_release_brake();
  HAL_TIM_PWM_Start(&htim1, AMS_TIM_CHANNELx);
#if USE_OF_ADC  
  if ( HAL_ADC_Start_DMA(&hadc1,(uint32_t *)aADCxConvertedData,(uint32_t)  64) != HAL_OK)
  {
    return false;
  } 
#endif  /* USE_OF_ADC */
  return true;
}

/**
 * @brief  Start motor in Down Direction
 * @param  None
 * @retval State
 */
bool ams_start_motor_down(void)
{
  HAL_GPIO_WritePin(AMS_DIR_GPIO_Port, AMS_DIR_PIN, GPIO_PIN_RESET ^ REVERSE_MOTOR_DIR);         //Arduino D12 = Direction	
  ams_release_brake();
  HAL_TIM_PWM_Start(&htim1, AMS_TIM_CHANNELx);
#if USE_OF_ADC   
  if ( HAL_ADC_Start_DMA(&hadc1,(uint32_t *)aADCxConvertedData,(uint32_t)  64) != HAL_OK)
  {
    return false;
  }
#endif  /* USE_OF_ADC */
  return true;
}

/**
 * @brief  Stop motor
 * @param  None
 * @retval State
 */
bool ams_stop_motor(void)
{
  HAL_TIM_PWM_Stop(&htim1, AMS_TIM_CHANNELx);
  ams_set_brake();
#if USE_OF_ADC    
  if ( HAL_ADC_Stop_DMA(&hadc1) != HAL_OK)
  {
    return false;
  }  
#endif  /* USE_OF_ADC */
  return true;
}

/**
 * @brief  Set Brake (Brake GPIO High)
 * @param  None
 * @retval None
 */
void ams_set_brake(void)
{
  HAL_GPIO_WritePin(AMS_RELEASE_BREAK_GPIO_Port, AMS_RELEASE_BREAK_PIN, GPIO_PIN_SET);
};

/**
 * @brief  Release Brake (Brake GPIO Low)
 * @param  None
 * @retval None
 */
void ams_release_brake(void)
{
  HAL_GPIO_WritePin(AMS_RELEASE_BREAK_GPIO_Port, AMS_RELEASE_BREAK_PIN, GPIO_PIN_RESET);
};