/**
  ******************************************************************************
  * @file    app_roller_shutter_window_covering.h
  * @author  Zigbee Application Team
  * @brief   Header for window covering cluster of the Roller Shutter Endpoint.
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
#ifndef APP_ROLLER_SHUTTER_WINDOW_COVERING_H
#define APP_ROLLER_SHUTTER_WINDOW_COVERING_H

#ifdef __cplusplus
extern "C" {
#endif

/* Defines ----------------------------------------------------------------- */
#define ZCL_COMMAND_ADC_STOP     0x09

/* Types ------------------------------------------------------------------- */
typedef struct
{
  struct ZigBeeT *zb; /* ZigBee stack reference. */

  // Window Covering variable
  uint8_t  window_cmd;

  /* Clusters used */
  struct ZbZclClusterT * window_server;
} Window_Cov_Control_T;

/* Exported Prototypes -------------------------------------------------------*/
Window_Cov_Control_T * App_Roller_Shutter_Window_Covering_Config(struct ZigBeeT *zb);
enum ZclStatusCodeT App_Roller_Shutter_Window_Covering_Restore_State  (void);

/* Set/Get Window command for cluster */
uint8_t App_Roller_Shutter_Window_Covering_Get_Cmd(void);
void    App_Roller_Shutter_Window_Covering_Set_Cmd(uint8_t window_cmd);

/* Window callbacks Definition ---------------------------------------------- */
enum ZclStatusCodeT Window_Server_Stop_Cb(struct ZbZclClusterT *cluster,
    struct ZbZclHeaderT *zclHdrPtr, struct ZbApsdeDataIndT *dataIndPtr, void *arg);
enum ZclStatusCodeT Window_Server_Up_Cb(struct ZbZclClusterT *cluster,
    struct ZbZclHeaderT *zclHdrPtr, struct ZbApsdeDataIndT *dataIndPtr, void *arg);
enum ZclStatusCodeT Window_Server_Down_Cb(struct ZbZclClusterT *cluster,
    struct ZbZclHeaderT *zclHdrPtr, struct ZbApsdeDataIndT *dataIndPtr, void *arg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_ROLLER_SHUTTER_WINDOW_COVERING_H */