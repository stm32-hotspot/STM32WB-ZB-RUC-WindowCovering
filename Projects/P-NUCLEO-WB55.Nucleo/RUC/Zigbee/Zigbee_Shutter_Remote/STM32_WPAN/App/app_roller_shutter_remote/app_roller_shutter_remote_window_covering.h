/**
  ******************************************************************************
  * @file    app_roller_shutter_remote_window_covering.h
  * @author  Zigbee Application Team
  * @brief   Header for Window covering cluster of the Roller Shutter Remote EndPoint.
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
#ifndef APP_ROLLER_SHUTTER_REMOTE_WINDOW_COVERING_H
#define APP_ROLLER_SHUTTER_REMOTE_WINDOW_COVERING_H

#ifdef __cplusplus
extern "C" {
#endif

/* Defines ----------------------------------------------------------------- */
#define ROLLER_SHUTTER_REMOTE_WINDOW_COVERING_MIN_REPORT            0x0000
#define ROLLER_SHUTTER_REMOTE_WINDOW_COVERING_MAX_REPORT            0x0000
#define ROLLER_SHUTTER_REMOTE_WINDOW_COVERING_REPORT_CHANGE         0x0001

// TODO use alarm cluster instead of command not use in window cov cluster
#define ZCL_COMMAND_ADC_STOP     0x09

/* Typedef ----------------------------------------------------------------- */
typedef struct
{
  bool is_init;
    
  /* Find&Bind part */
  uint8_t           bind_nb;
  struct ZbApsAddrT bind_table[NB_OF_SERV_BINDABLE];
  
  //WINDOW variable
  uint8_t cmd_send;
  uint8_t state;
  
  /* Clusters used */
  struct ZbZclClusterT * window_covering_client;
} Window_Cov_Control_T;

/* Exported Prototypes -------------------------------------------------------*/
Window_Cov_Control_T * App_Roller_Shutter_Remote_Window_Covering_ConfigEndpoint(struct ZigBeeT *zb);
void App_Roller_Shutter_Remote_Window_Covering_ReportConfig  (struct ZbApsAddrT * dst);
void App_Roller_Shutter_Remote_Window_Covering_Read_Attribute(struct ZbApsAddrT * dst);
void App_Roller_Shutter_Remote_Window_Covering_Cmd           (struct ZbApsAddrT * dst);

/* Window Covering Set/Get command -------------------------------------------*/
void    App_Roller_Shutter_Remote_Window_Covering_Set_Cmd  (uint8_t cmd_send);
uint8_t App_Roller_Shutter_Remote_Window_Covering_Get_Cmd  (void);
void    App_Roller_Shutter_Remote_Window_Covering_Set_state(uint8_t state);
uint8_t App_Roller_Shutter_Remote_Window_Covering_Get_state(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_ROLLER_SHUTTER_REMOTE_WINDOW_COVERING_H */