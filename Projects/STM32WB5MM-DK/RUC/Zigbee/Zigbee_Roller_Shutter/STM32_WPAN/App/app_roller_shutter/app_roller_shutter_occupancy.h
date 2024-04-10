/**
  ******************************************************************************
  * @file    app_roller_shutter_occupancy.h
  * @author  Zigbee Application Team
  * @brief   Header for Occupancy cluster application file for Roller shutter Endpoint.
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
#ifndef APP_ROLLER_SHUTTER_OCCUPANCY_H
#define APP_ROLLER_SHUTTER_OCCUPANCY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Defines ------------------------------------------------------------------ */
#define ROLLER_SHUTTER_OCCUPANCY_MIN_REPORT        0x0000
#define ROLLER_SHUTTER_OCCUPANCY_MAX_REPORT        0x0000
#define ROLLER_SHUTTER_OCCUPANCY_REPORT_CHANGE     0x0001 /* On/Off */


/* Types ------------------------------------------------------------------- */
typedef struct
{
  bool Occupancy;
  
  /* Clusters used */
  struct ZbZclClusterT * occupancy_client;
} Occupancy_Control_T;

/* Exported Prototypes -------------------------------------------------------*/
Occupancy_Control_T   * App_Roller_Shutter_Occupancy_Cfg          (struct ZigBeeT * zb);
enum ZclStatusCodeT     App_Roller_Shutter_Occupancy_Restore_State(void);
void                    App_Roller_Shutter_Occupancy_ReportConfig (struct ZbApsAddrT * dst);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_ROLLER_SHUTTER_OCCUPANCY_H */