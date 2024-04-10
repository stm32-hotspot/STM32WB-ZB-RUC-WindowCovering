/**
  ******************************************************************************
  * @file    app_light.h
  * @author  Zigbee Application Team
  * @brief   Header for Light application file.
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
#ifndef APP_LIGHT_H
#define APP_LIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Defines ----------------------------------------------------------------- */
/* Endpoint device */
#define LIGHT_ENDPOINT          0x0001U
#define LIGHT_GROUP_ADDR        0x0001U

/* Exported Prototypes -------------------------------------------------------*/
void App_Light_Cfg_Endpoint   (struct ZigBeeT *zb);
void App_Light_ConfigGroupAddr(void);
void App_Light_IdentifyMode   (void);
enum ZclStatusCodeT App_Light_Restore_State(void);
void App_Light_Bind_Disp      (void);
void App_Light_Toggle         (void);
void App_Light_Level_Up       (void);
void App_Light_Level_Down     (void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_LIGHT_H */