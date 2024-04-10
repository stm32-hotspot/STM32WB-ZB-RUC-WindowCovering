/**
  ******************************************************************************
  * @file    app_light_onoff.h
  * @author  Zigbee Application Team
  * @brief   Header for OnOff application file.
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
#ifndef APP_LIGHT_ONOFF_H 
#define APP_LIGHT_ONOFF_H 

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Defines ----------------------------------------------------------------- */

/* Types ------------------------------------------------------------------- */
typedef struct
{
  bool On;
    
  /* Clusters used */
  struct ZbZclClusterT * onoff_server;
} OnOff_Control_T;


/* Exported Prototypes -------------------------------------------------------*/
OnOff_Control_T   * App_Light_OnOff_Cfg(struct ZigBeeT *zb);
enum ZclStatusCodeT App_Light_OnOff_Restore_State(void);

// OnOff callback visible for the Endpoint
enum ZclStatusCodeT light_onoff_server_toggle_cb(
  struct ZbZclClusterT *clusterPtr, struct ZbZclAddrInfoT *srcInfo, void *arg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_LIGHT_ONOFF_H  */