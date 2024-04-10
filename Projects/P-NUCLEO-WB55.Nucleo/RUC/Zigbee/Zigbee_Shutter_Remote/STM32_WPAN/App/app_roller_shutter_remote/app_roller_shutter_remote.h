/**
  ******************************************************************************
  * @file    app_roller_shutter_remote.h
  * @author  Zigbee Application Team
  * @brief   Header of Roller Shutter Remote EndPoint application
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
#ifndef APP_ROLLER_SHUTTER_REMOTE_H
#define APP_ROLLER_SHUTTER_REMOTE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Defines ----------------------------------------------------------------- */
/* Endpoint device */
#define ROLLER_SHUTTER_REMOTE_ENDPOINT                  8U
#define ROLLER_SHUTTER_REMOTE_GROUP_ADDR           0x0008U

/* Retry */  
#define NB_OF_SERV_BINDABLE                             5
#define SIZE_RETRY_TAB            NB_OF_SERV_BINDABLE + 1
#define RETRY_ERROR_INDEX             NB_OF_SERV_BINDABLE
#define MAX_RETRY_REPORT                                5
#define MAX_RETRY_READ                                  5
#define MAX_RETRY_CMD                                   5

/* Typedef ----------------------------------------------------------------- */
typedef enum
{
  IDLE,
  FIND_AND_BIND,
  REPORT_CONF_WINDOW_ATTR,
  READ_WINDOW_ATTR,
  WRITE_WINDOW_ATTR,
} Cmd_Type_T;
      
typedef struct 
{
  Cmd_Type_T          cmd_type;
  struct ZbApsAddrT * dst;
} Cmd_Retry_T;

typedef struct 
{
  uint8_t  retry_nb;
  uint64_t ext_addr;
} Retry_number_T;

/* Exported Prototypes -------------------------------------------------------*/
void App_Roller_Shutter_Remote_ConfigEndpoint (struct ZigBeeT *zb);
void App_Roller_Shutter_Remote_ConfigGroupAddr(void);
void App_Roller_Shutter_Remote_Restore_State  (void);

void App_Roller_Shutter_Remote_FindBind       (void);
void App_Roller_Shutter_Remote_Bind_Disp      (void);

/* Retry function Declaration ----------------------------------------------- */
uint8_t * Get_retry_nb                       (Retry_number_T retry_tab[SIZE_RETRY_TAB], uint64_t server_addr);
void      App_Roller_Shutter_Remote_Retry_Cmd(Cmd_Type_T cmd, const struct ZbApsAddrT * dst);

/* Window control device -----------------------------------------------------*/
void App_Roller_Shutter_Remote_Move_Up  (void);
void App_Roller_Shutter_Remote_Move_Down(void);
void App_Roller_Shutter_Remote_Move_Stop(void);

void App_Roller_Shutter_Remote_Led_Blink(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_ROLLER_SHUTTER_REMOTE_H */
