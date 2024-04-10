/**
  ******************************************************************************
  * @file    stm32wbxx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32wbxx_it.h"

/* Private includes ----------------------------------------------------------*/
#include "stm32wb5mm_dk.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
extern void END_SENSOR_BOT_EXTI_Callback(void);
/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
extern IPCC_HandleTypeDef hipcc;
extern DMA_HandleTypeDef  hdma_usart1_tx;
extern UART_HandleTypeDef huart1;

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/* STM32WBxx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32wbxx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles PVD/PVM0/PVM2 interrupts through EXTI lines 16/31/33.
  */
void PVD_PVM_IRQHandler(void)
{
  HAL_PWREx_PVD_PVM_IRQHandler();
}

/**
  * @brief This function handles Flash global interrupt.
  */
void FLASH_IRQHandler(void)
{
  HAL_FLASH_IRQHandler();
}

/**
  * @brief This function handles RCC global interrupt.
  */
void RCC_IRQHandler(void)
{
}

/**
  * @brief This function handles CPU2 SEV interrupt through EXTI line 40 and PWR CPU2 HOLD wake-up interrupt.
  */
void C2SEV_PWR_C2H_IRQHandler(void)
{
}

/**
 * @brief This function handles USART1 global interrupt.
 */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}

/**
 * @brief This function handles HSEM global interrupt.
 */
void HSEM_IRQHandler(void)
{
  HAL_HSEM_IRQHandler();
}

/**
 * @brief This function handles DMA2 channel4 global interrupt.
 */
void DMA2_Channel4_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
}

/**
 * @brief This function handles PWR switching on the fly, end of BLE activity, end of 802.15.4 activity, end of critical radio phase interrupt.
 */
void PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler(void)
{
}

/**
 * @brief This function handles IPCC RX occupied interrupt.
 */
void IPCC_C1_RX_IRQHandler(void)
{
  HAL_IPCC_RX_IRQHandler(&hipcc);
}

/**
 * @brief This function handles IPCC TX free interrupt.
 */
void IPCC_C1_TX_IRQHandler(void)
{
  HAL_IPCC_TX_IRQHandler(&hipcc);
}

extern void END_SENSOR_BOT_EXTI_Callback(void);

/**
 * @brief  This function handles External line
 *         interrupt request.
 * @param  None
 * @retval None
 */
void PUSH_BUTTON_SW_EXTI_IRQHandler(void)
{
  BSP_PB_IRQHandler(BUTTON_USER1);
  BSP_PB_IRQHandler(BUTTON_USER2);
}

/**
 * @brief  This function handles TIM17 IRQ Handler.
 * @param  None
 * @retval None
 */
void TIM1_TRG_COM_TIM17_IRQHandler(void)
{
  BSP_PWM_LED_IRQHandler();
}

/**
 * @brief This function handles FPU global interrupt.
 */
void FPU_IRQHandler(void)
{
}

/* USER CODE BEGIN 1 */
void RTC_WKUP_IRQHandler(void)
{
  HW_TS_RTC_Wakeup_Handler();
}

/* USER CODE END 1 */