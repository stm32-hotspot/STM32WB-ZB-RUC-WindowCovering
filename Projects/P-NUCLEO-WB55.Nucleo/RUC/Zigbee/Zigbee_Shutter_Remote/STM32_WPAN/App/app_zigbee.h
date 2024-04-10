/**
  ******************************************************************************
  * @file    app_zigbee.h
  * @author  Zigbee Application Team
  * @brief   Header for Zigbee Application
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
#ifndef APP_ZIGBEE_H
#define APP_ZIGBEE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "zigbee_interface.h"

/* Private includes ----------------------------------------------------------*/
#include "tl.h"

/* Defines ------------------------------------------------------------------ */
/* Control the Tx power of the device */
#define TX_POWER_MIN  -20
#define TX_POWER_MAX    6
#define TX_POWER_STEP   1


/* Exported types ------------------------------------------------------------*/

/*
 *  List of all errors tracked by the Zigbee application
 *  running on M4. Some of these errors may be fatal
 *  or just warnings
 */
typedef enum
{
  ERR_REC_MULTI_MSG_FROM_M0,
  ERR_ZIGBE_CMD_TO_M0,
  ERR_ZIGBEE_CHECK_WIRELESS
} ErrAppliIdEnum_t;

/* zigbee app info structure */
typedef struct
{
  /* Zigbee stack */
  struct ZigBeeT * zb;

  /* Network infos */
  enum ZbStatusCodeT join_status;
  uint32_t           join_delay;
  int8_t             tx_power;  
} App_Zb_Info_T;


/* Exported functions ------------------------------------------------------- */
void App_Zigbee_Init                (void);
void App_Zigbee_Check_Firmware_Info (void);
void App_Zigbee_StackLayersInit     (void);
void App_Zigbee_Error               (uint32_t ErrId, uint32_t ErrCode);
void App_Zigbee_RegisterCmdBuffer   (TL_CmdPacket_t *p_buffer);
void App_Zigbee_ProcessNotifyM0ToM4 (void);
void App_Zigbee_ProcessRequestM0ToM4(void);
void App_Zigbee_TL_INIT             (void);
void Pre_ZigbeeCmdProcessing        (void);

/* Zigbee Network functions ------------------------------------------------- */
void App_Zigbee_Channel_Disp        (void);
void App_Zigbee_All_Address_Disp    (void);
void App_Zigbee_TxPwr_Up            (void);
void App_Zigbee_TxPwr_Down          (void);
void App_Zigbee_TxPwr_Disp          (void);
void App_Zigbee_Unbind_All          (void);
void App_Zigbee_Bind_Disp           (void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_ZIGBEE_H */

