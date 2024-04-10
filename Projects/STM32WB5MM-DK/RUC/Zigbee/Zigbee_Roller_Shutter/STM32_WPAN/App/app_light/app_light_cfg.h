/**
  ******************************************************************************
  * @file    app_light_cfg.h
  * @author  Zigbee Application Team
  * @brief   Header for configuration Light application file.
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
#ifndef APP_LIGHT_CFG_H
#define APP_LIGHT_CFG_H

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
#include "zcl/general/zcl.onoff.h"
#include "zcl/general/zcl.level.h"
// #include "zcl/general/zcl.occupancy.h"

/* EndPoint dependencies */
#include "app_light.h"
#include "app_light_onoff.h"
#include "app_light_level.h"
// #include "app_light_occupancy.h"

/* Types ------------------------------------------------------------------- */
typedef struct
{
  struct ZigBeeT *zb; /* ZigBee stack reference. */

  /* Clusters used */
  struct ZbZclClusterT * identify_server;
  // struct ZbZclClusterT * identify_client;
  OnOff_Control_T     * app_OnOff;
  Level_Control_T     * app_Level;
  // Occupancy_Control_T * app_Occupancy;

  /* Blinkt LED bar graph control */
  uint8_t Led_Lvl;

} Light_Control_T;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_LIGHT_CFG_H */