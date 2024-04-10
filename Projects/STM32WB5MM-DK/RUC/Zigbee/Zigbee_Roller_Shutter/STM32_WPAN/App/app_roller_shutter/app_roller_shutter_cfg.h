/**
  ******************************************************************************
  * @file    app_roller_shutter_cfg.h
  * @author  Zigbee Application Team
  * @brief   Header configuration of Roller Shutter Endpoint application file.
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
#ifndef APP_ROLLER_SHUTTER_CFG_H
#define APP_ROLLER_SHUTTER_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "app_entry.h"
#include "zigbee_interface.h"

/* Debug Part */
#include "stm_logging.h"
#include "dbg_trace.h"
#include <assert.h>

/* Zigbee Cluster */
#include "zcl/zcl.h"
#include "zcl/general/zcl.identify.h"
#include "zcl/general/zcl.window.h"
#include "zcl/general/zcl.occupancy.h"

/* EndPoint dependencies */
#include "app_roller_shutter_window_covering.h"
#include "app_roller_shutter_occupancy.h"
#include "app_roller_shutter.h"

/* Typedef ------------------------------------------------------------------*/
typedef struct
{
  struct ZigBeeT *zb; /* ZigBee stack reference. */

  /* Cluster Parts */
  struct ZbZclClusterT * identify_server;
  Window_Cov_Control_T * app_Window_Covering_Control;
  Occupancy_Control_T  * app_Occupancy;

  // The Shutter FSM
  Shutter_State_T state;

  // Roller Shutter variable
  uint16_t secure_timer_up;       // in ms
  uint16_t secure_timer_down;     // in ms
  uint32_t PWM_Motor_Speed;       // in ms

  // Shutter detection
  uint32_t ADC_TresholdHigh_Up;   // in ms
  uint32_t ADC_TresholdHigh_Down; // in ms
  uint32_t ADC_TresholdLow;       // in ms  
} Roller_Shutter_Control_T;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_ROLLER_SHUTTER_CFG_H */