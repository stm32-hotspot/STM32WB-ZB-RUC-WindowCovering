/**
  ******************************************************************************
  * @file    stm32wbxx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32WBxx_IT_H
#define __STM32WBxx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
#include "app_common.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void PVD_PVM_IRQHandler(void);
void FLASH_IRQHandler(void);
void RCC_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);
void C2SEV_PWR_C2H_IRQHandler(void);
void USART1_IRQHandler(void);
void LPUART1_IRQHandler(void);
void PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler(void);
void HSEM_IRQHandler(void);
void FPU_IRQHandler(void);
void RTC_WKUP_IRQHandler(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI4_IRQHandler(void);
void IPCC_C1_TX_IRQHandler(void);
void IPCC_C1_RX_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32WBxx_IT_H */
