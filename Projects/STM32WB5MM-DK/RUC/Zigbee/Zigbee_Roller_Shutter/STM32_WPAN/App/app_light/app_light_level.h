/**
  ******************************************************************************
  * @file    app_light_level.h
  * @author  Zigbee Application Team
  * @brief   Header for Level cluster application file for Light EndPoint.
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
#ifndef APP_LIGHT_LEVEL_H
#define APP_LIGHT_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Defines ----------------------------------------------------------------- */
#define LIGHT_LEVEL_MAX_VALUE           0xFFU
#define LIGHT_LEVEL_MIN_VALUE           0x0FU
#define LIGHT_LEVEL_STEP                0x10U
#define LIGHT_LEVEL_DEFAULT             0x1FU

#define LIGHT_LEVEL_MODE_UP                1U
#define LIGHT_LEVEL_MODE_DOWN              0U

/* Types ------------------------------------------------------------------- */
typedef struct
{
  uint8_t level;
  bool    level_inc;
  
  /* Clusters used */
  struct ZbZclClusterT * level_server;
} Level_Control_T;

/* Exported Prototypes -------------------------------------------------------*/
Level_Control_T   * App_Light_Level_Cfg(struct ZigBeeT *zb);
enum ZclStatusCodeT App_Light_Level_Restore_State(void);

enum ZclStatusCodeT light_level_server_move_to_level_cb(struct ZbZclClusterT *clusterPtr, struct ZbZclLevelClientMoveToLevelReqT *req, struct ZbZclAddrInfoT *srcInfo, void *arg);
enum ZclStatusCodeT light_level_server_move_cb(struct ZbZclClusterT *clusterPtr, struct ZbZclLevelClientMoveReqT *req, struct ZbZclAddrInfoT *srcInfo, void *arg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_LIGHT_LEVEL_H */