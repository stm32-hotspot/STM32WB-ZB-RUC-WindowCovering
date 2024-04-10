/**
 ******************************************************************************
 * @file    AMS_PWM.c
 * @author  MCD Application Team
 * @brief   PWM config for Arduino Motor Shield driver
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
#include "AMS_PWM.h"

TIM_HandleTypeDef htim1;

/**
 * @brief  Initializes PWM1 for AMS
 * @param  duty_cycle : Value should be between 0 and 100, 100 means 100% of 3.3V
 * @retval State
 */
bool ams_pwm_Init(uint32_t duty_cycle)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};


  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;                 //Prescaler = 0 & Pulse , periode = 1 ms
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

/* -------------------  HAL_TIM_PWM_Init  ----------------------------- */
  if (htim1.State == HAL_TIM_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    htim1.Lock = HAL_UNLOCKED;

    /* Init the low level hardware : GPIO, CLOCK, NVIC and DMA */
    /* Peripheral clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();
    /* TIM1 interrupt Init */
    HAL_NVIC_SetPriority(AMS_TIMx_CC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(AMS_TIMx_CC_IRQn);
  }
  
  /* Set the TIM state */
  htim1.State = HAL_TIM_STATE_BUSY;
  /* Init the base time for the PWM */
  TIM_Base_SetConfig(htim1.Instance, &htim1.Init);
  /* Initialize the DMA burst operation state */
  htim1.DMABurstState = HAL_DMA_BURST_STATE_READY;
  /* Initialize the TIM channels state */
  TIM_CHANNEL_STATE_SET_ALL(&htim1, HAL_TIM_CHANNEL_STATE_READY);
  TIM_CHANNEL_N_STATE_SET_ALL(&htim1, HAL_TIM_CHANNEL_STATE_READY);
  /* Initialize the TIM state*/
  htim1.State = HAL_TIM_STATE_READY;

/* ------------------- End HAL_TIM_PWM_Init  ----------------------------- */
  
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    return false;
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 65535 * duty_cycle/100;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, AMS_TIM_CHANNELx) != HAL_OK)
  {
    return false;
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    return false;
  }
  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(htim1.Instance == TIM1)
  {
    ENABLE_GPIOx_CLK(AMS_PWM_GPIO_Port);
    
    GPIO_InitStruct.Pin = AMS_PWM_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(AMS_PWM_GPIO_Port, &GPIO_InitStruct);
  }
  
  HAL_TIM_PWM_Stop(&htim1, AMS_TIM_CHANNELx);
  
  return true;
};

/**
 * @brief  Change PWM Duty cycle
 * @param  duty_cycle : Value should be between 0 and 100, 100 means 100% of 3.3V
 * @retval None
 */
void ams_pwm_change_duty_cycle(uint32_t duty_cycle)
{
  if (duty_cycle <= 100)
  {
    __HAL_TIM_SET_COMPARE(&htim1, AMS_TIM_CHANNELx, 65535 * duty_cycle/100);
  }
}

/**
 * @brief  PWM TIM1 IRQ Handler
 * @retval None
 */
void AMS_TIMx_CC_IRQHandler (void)
{
  HAL_TIM_IRQHandler(&htim1);
}