/**
 ******************************************************************************
 * @file    AMS_ADC.h
 * @author  MCD Application Team
 * @brief   ADC Header for Arduino Motor Shield-Rev3 driver
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
#ifndef AMS_ADC_H
#define AMS_ADC_H

#include "AMS_conf.h"

#ifdef __cplusplus
extern "C" {
#endif


#define ADC_CFGR_FIELDS_1  ((ADC_CFGR_RES  | ADC_CFGR_ALIGN   |\
                           ADC_CFGR_CONT   | ADC_CFGR_OVRMOD  |\
                           ADC_CFGR_DISCEN | ADC_CFGR_DISCNUM |\
                           ADC_CFGR_EXTEN  | ADC_CFGR_EXTSEL))   /*!< ADC_CFGR fields of parameters that can be updated
                                                                      when no regular conversion is on-going */

/**
 * @brief  Initializes ADC for AMS
 * @param  HighThreshold : Value should be between 0 and 0xFFF, 0xFFF = 2A max supported = 3.3V in ADC GPIO
 * Value exemple :    2A =  3,3 V = 4095 = 0xfff (12 Bit resolution)
 *				   0.03A =  50 mV =   62 = 0x03E
 *				   0.05A = 82,5mV =  102 = 0x066
 *				   0.06A =  100mV =  124 = 0x07C
 * @retval HAL_StatusTypeDef : state
 */
HAL_StatusTypeDef ams_adc_init(uint32_t HighThreshold);

/**
 * @brief  Initializes DMA for ADC
 */
void ams_dma_Init(void);

/**
 * @brief  Change Max/Min Treshold value of ADC watchdog
 * @param  HighThreshold : Value should be between 0 and 0xFFF, 0xFFF = 2A max supported = 3.3V in ADC GPIO
 * @param  LowThreshold : Value should be between 0 and 0xFFF, 0xFFF = 2A max supported = 3.3V in ADC GPIO
 * @retval HAL_StatusTypeDef : state
 */
bool ams_adc_change_treshold_value(uint32_t HighThreshold,  uint32_t LowThreshold);

/**
 * @brief DMA IRQ handler for ADC watchdog
 */
void AMS_DMA1_CHANNELx_IRQHandler(void);

/**
 * @brief ADC IRQ handler
 */
void ADC1_IRQHandler(void);

/**
 * @brief Definition of Error Handler
 */
void adc_Error_Handler(void);

#endif 

#ifdef __cplusplus
}

#endif /* AMS_ADC_H */
