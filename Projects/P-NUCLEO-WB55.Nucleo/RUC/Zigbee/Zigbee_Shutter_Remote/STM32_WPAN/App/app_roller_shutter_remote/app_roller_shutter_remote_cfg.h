/**
  ******************************************************************************
  * @file    app_roller_shutter_remote_cfg.h
  * @author  Zigbee Application Team
  * @brief   Header of configuration for Roller Shutter Endpoint application file.
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
#ifndef APP_ROLLER_SHUTTER_REMOTE_CFG_H
#define APP_ROLLER_SHUTTER_REMOTE_CFG_H

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

/* EndPoint dependencies */
#include "app_roller_shutter_remote.h"
#include "app_roller_shutter_remote_window_covering.h"

/* Typedef ------------------------------------------------------------------*/
typedef struct
{
  struct ZigBeeT *zb; /* ZigBee stack reference. */

  /* Cluster Parts */
  struct ZbZclClusterT *identify_client;
  Window_Cov_Control_T *app_Window_Covering_Control;

  /* EndPoint Retry management */
  uint8_t     is_rdy_for_next_cmd;
  bool        force_read;
  Cmd_Retry_T cmd_retry;
} Shutter_Remote_T;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_ROLLER_SHUTTER_REMOTE_CFG_H */