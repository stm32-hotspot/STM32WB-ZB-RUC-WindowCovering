/**
 ******************************************************************************
 * @file    AMS_ADC.c
 * @author  MCD Application Team
 * @brief   ADC config for Arduino Motor Shield driver
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
#include "AMS_ADC.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/**
 * @brief  Initializes ADC for AMS
 * @param  HighThreshold : Value should be between 0 and 0xFFF, 0xFFF = 2A max supported = 3.3V in ADC GPIO
 * Value exemple :    2A =  3,3 V = 4095 = 0xfff (12 Bit resolution)
 *				   0.03A =  50 mV =   62 = 0x03E
 *				   0.05A = 82,5mV =  102 = 0x066
 *				   0.06A =  100mV =  124 = 0x07C
 * @retval HAL_StatusTypeDef : state
 */
HAL_StatusTypeDef ams_adc_init(uint32_t HighThreshold)
{

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.OversamplingMode = DISABLE;

  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  
  uint32_t tmpCFGR = 0UL;
  uint32_t tmp_adc_reg_is_conversion_on_going;
  __IO uint32_t wait_loop_index = 0UL;

  uint32_t tmp_adc_is_conversion_on_going_regular;
  uint32_t tmp_adc_is_conversion_on_going_injected;

  if (hadc1.Init.ScanConvMode != ADC_SCAN_DISABLE)
  {
    assert_param(IS_ADC_REGULAR_NB_CONV(hadc->Init.NbrOfConversion));
    assert_param(IS_FUNCTIONAL_STATE(hadc->Init.DiscontinuousConvMode));

    if (hadc1.Init.DiscontinuousConvMode == ENABLE)
    {
      assert_param(IS_ADC_REGULAR_DISCONT_NUMBER(hadc->Init.NbrOfDiscConversion));
    }
  }

  /* DISCEN and CONT bits cannot be set at the same time */
  assert_param(!((hadc->Init.DiscontinuousConvMode == ENABLE) && (hadc->Init.ContinuousConvMode == ENABLE)));

  

  /* Actions performed only if ADC is coming from state reset:                */
  /* - Initialization of ADC MSP                                              */
  if (hadc1.State == HAL_ADC_STATE_RESET)
  {

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};


    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      adc_Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();

    ENABLE_GPIOx_CLK(AMS_ADC_GPIO_Port)
    /**ADC1 GPIO Configuration
    */
    GPIO_InitStruct.Pin = AMS_ADC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(AMS_ADC_GPIO_Port, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = AMS_DMA1_CHANNELx;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      adc_Error_Handler();
    }

    __HAL_LINKDMA(&hadc1,DMA_Handle,hdma_adc1);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC1_IRQn);

    /* Set ADC error code to none */
    ADC_CLEAR_ERRORCODE(&hadc1);

    /* Initialize Lock */
    hadc1.Lock = HAL_UNLOCKED;
  }

  /* - Exit from deep power-down mode and ADC voltage regulator enable        */

  if (LL_ADC_IsDeepPowerDownEnabled(hadc1.Instance) != 0UL)
  {
    /* Disable ADC deep power down mode */
    LL_ADC_DisableDeepPowerDown(hadc1.Instance);

    /* System was in deep power down mode, calibration must
     be relaunched or a previously saved calibration factor
     re-applied once the ADC voltage regulator is enabled */
  }


  if (LL_ADC_IsInternalRegulatorEnabled(hadc1.Instance) == 0UL)
  {
    /* Enable ADC internal voltage regulator */
    LL_ADC_EnableInternalRegulator(hadc1.Instance);

    /* Note: Variable divided by 2 to compensate partially              */
    /*       CPU processing cycles, scaling in us split to not          */
    /*       exceed 32 bits register capacity and handle low frequency. */
    wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US / 10UL) * ((SystemCoreClock / (100000UL * 2UL)) + 1UL));
    while (wait_loop_index != 0UL)
    {
      wait_loop_index--;
    }
  }

  /* Verification that ADC voltage regulator is correctly enabled, whether    */
  /* or not ADC is coming from state reset (if any potential problem of       */
  /* clocking, voltage regulator would not be enabled).                       */
  if (LL_ADC_IsInternalRegulatorEnabled(hadc1.Instance) == 0UL)
  {
    /* Update ADC state machine to error */
    SET_BIT(hadc1.State, HAL_ADC_STATE_ERROR_INTERNAL);

    /* Set ADC error code to ADC peripheral internal error */
    SET_BIT(hadc1.ErrorCode, HAL_ADC_ERROR_INTERNAL);

    tmp_hal_status = HAL_ERROR;
  }

  /* Configuration of ADC parameters if previous preliminary actions are      */
  /* correctly completed and if there is no conversion on going on regular    */
  /* group (ADC may already be enabled at this point if HAL_ADC_Init() is     */
  /* called to update a parameter on the fly).                                */
  tmp_adc_reg_is_conversion_on_going = LL_ADC_REG_IsConversionOngoing(hadc1.Instance);

  if (((hadc1.State & HAL_ADC_STATE_ERROR_INTERNAL) == 0UL)
      && (tmp_adc_reg_is_conversion_on_going == 0UL)
     )
  {
    /* Set ADC state */
    ADC_STATE_CLR_SET(hadc1.State,
                      HAL_ADC_STATE_REG_BUSY,
                      HAL_ADC_STATE_BUSY_INTERNAL);

    /* Configuration of common ADC parameters                                 */

    /* Parameters update conditioned to ADC state:                            */
    /* Parameters that can be updated only when ADC is disabled:              */
    /*  - clock configuration                                                 */
    if (LL_ADC_IsEnabled(hadc1.Instance) == 0UL)
    {
      if (__LL_ADC_IS_ENABLED_ALL_COMMON_INSTANCE(__LL_ADC_COMMON_INSTANCE(hadc1->Instance)) == 0UL)
      {
        /* Reset configuration of ADC common register CCR:                      */
        /*                                                                      */
        /*   - ADC clock mode and ACC prescaler (CKMODE and PRESC bits)are set  */
        /*     according to adc->Init.ClockPrescaler. It selects the clock      */
        /*    source and sets the clock division factor.                        */
        /*                                                                      */
        /* Some parameters of this register are not reset, since they are set   */
        /* by other functions and must be kept in case of usage of this         */
        /* function on the fly (update of a parameter of ADC_InitTypeDef        */
        /* without needing to reconfigure all other ADC groups/channels         */
        /* parameters):                                                         */
        /*   - when multimode feature is available, multimode-related           */
        /*     parameters: MDMA, DMACFG, DELAY, DUAL (set by API                */
        /*     HAL_ADCEx_MultiModeConfigChannel() )                             */
        /*   - internal measurement paths: Vbat, temperature sensor, Vref       */
        /*     (set into HAL_ADC_ConfigChannel() or                             */
        /*     HAL_ADCEx_InjectedConfigChannel() )                              */
        LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(hadc1->Instance), hadc1.Init.ClockPrescaler);
      }
    }

    /* Configuration of ADC:                                                  */
    /*  - resolution                               Init.Resolution            */
    /*  - data alignment                           Init.DataAlign             */
    /*  - external trigger to start conversion     Init.ExternalTrigConv      */
    /*  - external trigger polarity                Init.ExternalTrigConvEdge  */
    /*  - continuous conversion mode               Init.ContinuousConvMode    */
    /*  - overrun                                  Init.Overrun               */
    /*  - discontinuous mode                       Init.DiscontinuousConvMode */
    /*  - discontinuous mode channel count         Init.NbrOfDiscConversion   */
    tmpCFGR |= (ADC_CFGR_CONTINUOUS((uint32_t)hadc1.Init.ContinuousConvMode)           |
                hadc1.Init.Overrun                                                     |
                hadc1.Init.DataAlign                                                   |
                hadc1.Init.Resolution                                                  |
                ADC_CFGR_REG_DISCONTINUOUS((uint32_t)hadc1.Init.DiscontinuousConvMode));

    if (hadc1.Init.DiscontinuousConvMode == ENABLE)
    {
      tmpCFGR |= ADC_CFGR_DISCONTINUOUS_NUM(hadc1.Init.NbrOfDiscConversion);
    }

    /* Enable external trigger if trigger selection is different of software  */
    /* start.                                                                 */
    /* Note: This configuration keeps the hardware feature of parameter       */
    /*       ExternalTrigConvEdge "trigger edge none" equivalent to           */
    /*       software start.                                                  */
    if (hadc1.Init.ExternalTrigConv != ADC_SOFTWARE_START)
    {
      tmpCFGR |= ((hadc1.Init.ExternalTrigConv & ADC_CFGR_EXTSEL)
                  | hadc1.Init.ExternalTrigConvEdge
                 );
    }

    /* Update Configuration Register CFGR */
    MODIFY_REG(hadc1.Instance->CFGR, ADC_CFGR_FIELDS_1, tmpCFGR);

    /* Parameters update conditioned to ADC state:                            */
    /* Parameters that can be updated when ADC is disabled or enabled without */
    /* conversion on going on regular and injected groups:                    */
    /*  - DMA continuous request          Init.DMAContinuousRequests          */
    /*  - LowPowerAutoWait feature        Init.LowPowerAutoWait               */
    /*  - Oversampling parameters         Init.Oversampling                   */
    tmp_adc_is_conversion_on_going_regular = LL_ADC_REG_IsConversionOngoing(hadc1.Instance);
    tmp_adc_is_conversion_on_going_injected = LL_ADC_INJ_IsConversionOngoing(hadc1.Instance);
    if ((tmp_adc_is_conversion_on_going_regular == 0UL)
        && (tmp_adc_is_conversion_on_going_injected == 0UL)
       )
    {
      tmpCFGR = (ADC_CFGR_DFSDM(hadc1)                                            |
                 ADC_CFGR_AUTOWAIT((uint32_t)hadc1.Init.LowPowerAutoWait)        |
                 ADC_CFGR_DMACONTREQ((uint32_t)hadc1.Init.DMAContinuousRequests));

      MODIFY_REG(hadc1.Instance->CFGR, ADC_CFGR_FIELDS_2, tmpCFGR);

      if (hadc1.Init.OversamplingMode == ENABLE)
      {
        assert_param(IS_ADC_OVERSAMPLING_RATIO(hadc1->Init.Oversampling.Ratio));
        assert_param(IS_ADC_RIGHT_BIT_SHIFT(hadc1->Init.Oversampling.RightBitShift));
        assert_param(IS_ADC_TRIGGERED_OVERSAMPLING_MODE(hadc1->Init.Oversampling.TriggeredMode));
        assert_param(IS_ADC_REGOVERSAMPLING_MODE(hadc1->Init.Oversampling.OversamplingStopReset));

        /* Configuration of Oversampler:                                      */
        /*  - Oversampling Ratio                                              */
        /*  - Right bit shift                                                 */
        /*  - Triggered mode                                                  */
        /*  - Oversampling mode (continued/resumed)                           */
        MODIFY_REG(hadc1.Instance->CFGR2,
                   ADC_CFGR2_OVSR  |
                   ADC_CFGR2_OVSS  |
                   ADC_CFGR2_TROVS |
                   ADC_CFGR2_ROVSM,
                   ADC_CFGR2_ROVSE                       |
                   hadc1.Init.Oversampling.Ratio         |
                   hadc1.Init.Oversampling.RightBitShift |
                   hadc1.Init.Oversampling.TriggeredMode |
                   hadc1.Init.Oversampling.OversamplingStopReset
                  );
      }
      else
      {
        /* Disable ADC oversampling scope on ADC group regular */
        CLEAR_BIT(hadc1.Instance->CFGR2, ADC_CFGR2_ROVSE);
      }

    }

    /* Configuration of regular group sequencer:                              */
    /* - if scan mode is disabled, regular channels sequence length is set to */
    /*   0x00: 1 channel converted (channel on regular rank 1)                */
    /*   Parameter "NbrOfConversion" is discarded.                            */
    /*   Note: Scan mode is not present by hardware on this device, but       */
    /*   emulated by software for alignment over all STM32 devices.           */
    /* - if scan mode is enabled, regular channels sequence length is set to  */
    /*   parameter "NbrOfConversion".                                         */

    if (hadc1.Init.ScanConvMode == ADC_SCAN_ENABLE)
    {
      /* Set number of ranks in regular group sequencer */
      MODIFY_REG(hadc1.Instance->SQR1, ADC_SQR1_L, (hadc1.Init.NbrOfConversion - (uint8_t)1));
    }
    else
    {
      CLEAR_BIT(hadc1.Instance->SQR1, ADC_SQR1_L);
    }

    /* Initialize the ADC state */
    /* Clear HAL_ADC_STATE_BUSY_INTERNAL bit, set HAL_ADC_STATE_READY bit */
    ADC_STATE_CLR_SET(hadc1.State, HAL_ADC_STATE_BUSY_INTERNAL, HAL_ADC_STATE_READY);
  }
  else
  {
    /* Update ADC state machine to error */
    SET_BIT(hadc1.State, HAL_ADC_STATE_ERROR_INTERNAL);

    tmp_hal_status = HAL_ERROR;
  }

  /* Return function status */
  if (tmp_hal_status != HAL_OK)
  {
    return tmp_hal_status;
  }


  ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
  ADC_ChannelConfTypeDef sConfig = {0};
  /** Configure Analog WatchDog 1
  */

  AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
  AnalogWDGConfig.Channel = AMS_ADC_CHANNEL;
  AnalogWDGConfig.ITMode = ENABLE;
  AnalogWDGConfig.HighThreshold = HighThreshold;
  AnalogWDGConfig.LowThreshold = 0;
  if (HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) != HAL_OK)
  {
    return false;
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = AMS_ADC_CHANNEL;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    return false;
  }
  return true;
}

/**
 * @brief  Change Max/Min Treshold value of ADC watchdog
 * @param  HighThreshold : Value should be between 0 and 0xFFF, 0xFFF = 2A max supported = 3.3V in ADC GPIO
 * @param  LowThreshold : Value should be between 0 and 0xFFF, 0xFFF = 2A max supported = 3.3V in ADC GPIO
 * @retval HAL_StatusTypeDef : state
 */
bool ams_adc_change_treshold_value(uint32_t HighThreshold,  uint32_t LowThreshold){
  LL_ADC_ConfigAnalogWDThresholds(hadc1.Instance, ADC_ANALOGWATCHDOG_1, HighThreshold , LowThreshold);
  return true;
}

/**
  * @brief  Analog watchdog callback in non blocking mode.
  * @param  hadc: ADC handle
  * @retval None
  */
__weak void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);

  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_LevelOutOfWindowCallback must be implemented in the user file.
  */
}

/**
 * @brief  Initializes DMA for ADC
 */
void ams_dma_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();  

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(AMS_DMA1_Channelx_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(AMS_DMA1_Channelx_IRQn);
  
}

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void AMS_DMA1_CHANNELx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc1);
}

/**
  * @brief This function handles ADC1 global interrupt.
  */
void ADC1_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&hadc1);
}

/**
 * @brief Definition of Error Handler
 */
void adc_Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}