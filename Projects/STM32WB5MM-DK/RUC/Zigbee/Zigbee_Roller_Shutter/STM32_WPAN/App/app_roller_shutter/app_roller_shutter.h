/**
  ******************************************************************************
  * @file    app_roller_shutter.h
  * @author  Zigbee Application Team
  * @brief   Header for Roller Shutter Endpoint application file.
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
#ifndef APP_ROLLER_SHUTTER_H
#define APP_ROLLER_SHUTTER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------- */
#include "app_common.h"
#include "app_entry.h"
#include "zigbee_interface.h"

#include "stm_logging.h"
#include "dbg_trace.h"
#include <assert.h>

/* Zigbee Cluster */
#include "zcl/zcl.h"
#include "zcl/general/zcl.identify.h"
#include "zcl/general/zcl.window.h"
#include "AMS.h"

/* Defines ----------------------------------------------------------------- */
/* Endpoint device */
#define ROLLER_SHUTTER_ENDPOINT         0x0008U
#define ROLLER_SHUTTER_GROUP_ADDR       0x0008U

/* Motor Control default values */
#define DEFAULT_secure_timer_up            3000U
#define DEFAULT_secure_timer_down          3000U
#define SECURE_TIMER_STEP                   250U
#define DEFAULT_PWM_Motor_Speed             100U
#define PWM_MOTOR_SPEED_STEP                  5U
#define DEFAULT_ADC_TresholdHigh_Up         190U
#define DEFAULT_ADC_TresholdHigh_Down       250U
#define DEFAULT_ADC_TresholdLow               0U
#define ADC_TRESHOLD_STEP                     5U

/* Limit Switch Top GPIO Config */
#define LIMIT_SWITCH_TOP_PIN                   GPIO_PIN_4
#define LIMIT_SWITCH_TOP_GPIO_Port             GPIOE
#define LIMIT_SWITCH_TOP_EXTIx_IRQHandler      EXTI4_IRQHandler
#define LIMIT_SWITCH_TOP_EXTIx_IRQn            EXTI4_IRQn
void LIMIT_SWITCH_TOP_EXTIx_IRQHandler(void);

/* Limit Switch Bottom GPIO Config */
#define LIMIT_SWITCH_BOT_PIN                   GPIO_PIN_5
#define LIMIT_SWITCH_BOT_GPIO_Port             GPIOC
#define LIMIT_SWITCH_BOT_EXTIx_IRQHandler      EXTI9_5_IRQHandler
#define LIMIT_SWITCH_BOT_EXTIx_IRQn            EXTI9_5_IRQn
void LIMIT_SWITCH_BOT_EXTIx_IRQHandler(void);

/* Window Bottom delay */
#define BOTTOM_END_DELAY                       300U
#define HW_TS_BOTTOM_END_DELAY                 (BOTTOM_END_DELAY * HW_TS_SERVER_1ms_NB_TICKS)  /**< 0.5s */

/* Types ------------------------------------------------------------------- */
typedef enum 
{
  IDLE,
  RUN,
  TOP_REACHED, 
  BOTTOM_REACHED,
} Shutter_State_T;

/* Exported Prototypes -------------------------------------------------------*/
void App_Roller_Shutter_Cfg_Endpoint                (struct ZigBeeT *zb);
enum ZclStatusCodeT App_Roller_Shutter_Restore_State(void);
void App_Roller_Shutter_IdentifyMode                (void);
void App_Roller_Shutter_FindBind                    (void);
void App_Roller_Shutter_Bind_Disp                   (void);

/* Roller Shutter Motor Control */
void App_Roller_Shutter_Stop                (void);
void App_Roller_Shutter_Up                  (void);
void App_Roller_Shutter_Down                (void);
void App_Roller_Shutter_timer_motorup_up    (void);
void App_Roller_Shutter_timer_motorup_down  (void);
void App_Roller_Shutter_timer_motordown_up  (void);
void App_Roller_Shutter_timer_motordown_down(void);
void App_Roller_Shutter_motor_speed_up      (void);
void App_Roller_Shutter_motor_speed_down    (void);
void App_Roller_Shutter_adc_treshold_up     (void);
void App_Roller_Shutter_adc_treshold_down   (void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_ROLLER_SHUTTER_H */