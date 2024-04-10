/**
  ******************************************************************************
  * @file    app_light.c
  * @author  Zigbee Application Team
  * @brief   Application interface for Light EndPoint functionnalities
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

/* Includes ------------------------------------------------------------------*/
#include "stm32_seq.h"

/* board dependancies */
#include "stm32wb5mm_dk_lcd.h"
#include "stm32_lcd.h"

/* HW dependancies*/
#include "blinkt.h"

/* service dependancies */
#include "app_light_cfg.h"
#include "app_core.h"
#include "app_zigbee.h"

/* Private defines -----------------------------------------------------------*/
#define IDENTIFY_MODE_DELAY              30
#define HW_TS_IDENTIFY_MODE_DELAY         (IDENTIFY_MODE_DELAY * HW_TS_SERVER_1S_NB_TICKS)

/* Application Variable-------------------------------------------------------*/
extern App_Zb_Info_T app_zb_info;
extern bool id_mode_on;

Light_Control_T app_Light_Control =
{
  .Led_Lvl = 0x20U,
};

/* Private function ----------------------------------------------------------*/
/* Blinkt Bar LEDs */
static void App_Light_Blinkt_GPIO_Init(void);
static void App_Light_Control_Task    (void);

/* Finding&Binding function Declaration --------------------------------------*/
static void App_Light_Identify_cb(struct ZbZclClusterT *cluster, enum ZbZclIdentifyServerStateT state, void *arg);


/* Clusters CFG ------------------------------------------------------------ */
/**
 * @brief  Configure and register Zigbee application endpoints, onoff callbacks
 * @param  zb Zigbee stack instance
 * @retval None
 */
void App_Light_Cfg_Endpoint(struct ZigBeeT *zb)
{
  struct ZbApsmeAddEndpointReqT  req;
  struct ZbApsmeAddEndpointConfT conf;
  
  /* Endpoint: LIGHT_ENDPOINT */
  memset(&req, 0, sizeof(req));
  req.profileId = ZCL_PROFILE_HOME_AUTOMATION;
  req.deviceId  = ZCL_DEVICE_ONOFF_LIGHT;
  req.endpoint  = LIGHT_ENDPOINT;
  ZbZclAddEndpoint(zb, &req, &conf);
  assert(conf.status == ZB_STATUS_SUCCESS);

  /* Idenfity server */
  app_Light_Control.identify_server = ZbZclIdentifyServerAlloc(zb, LIGHT_ENDPOINT, NULL);
  assert(app_Light_Control.identify_server != NULL);
  assert(ZbZclClusterEndpointRegister(app_Light_Control.identify_server)!= NULL);
  ZbZclIdentifyServerSetCallback(app_Light_Control.identify_server, App_Light_Identify_cb);

  /* OnOff Server */
  app_Light_Control.app_OnOff = App_Light_OnOff_Cfg(zb);

  /*  Level Server */
  app_Light_Control.app_Level = App_Light_Level_Cfg(zb);

  /*  Occupancy Client */
  // app_Light_Control.app_Occupancy = App_Light_Occupancy_Cfg(zb);

  /* make a local pointer of Zigbee stack to read easier */
  app_Light_Control.zb = zb;

  /* blinkt LED bar graph init -----------------------------------------------*/
  App_Light_Blinkt_GPIO_Init();
  
  /* BAR LED control task */
  UTIL_SEQ_RegTask(1U << CFG_TASK_LIGHT_UPDATE, UTIL_SEQ_RFU, App_Light_Control_Task);
} /* App_Light_Cfg_Endpoint */

/**
 * @brief  Associate the Endpoint to the group address
 * @param  zb Zigbee stack instance
 * @retval None
 */
void App_Light_ConfigGroupAddr(void)
{
  struct ZbApsmeAddGroupReqT  req;
  struct ZbApsmeAddGroupConfT conf;
  
  memset(&req, 0, sizeof(req));
  req.endpt     = LIGHT_ENDPOINT;
  req.groupAddr = LIGHT_GROUP_ADDR;
  ZbApsmeAddGroupReq(app_Light_Control.zb, &req, &conf);
} /* App_Light_ConfigGroupAddr */

/* Light Persistence -------------------------------------------------------- */
/**
 * @brief  Restore the state at startup from persistence
 * @param  None
 * @retval stack status code
 */
enum ZclStatusCodeT App_Light_Restore_State(void)
{
  if (App_Light_OnOff_Restore_State() != ZCL_STATUS_SUCCESS)
    return ZCL_STATUS_FAILURE;
  if (App_Light_Level_Restore_State() != ZCL_STATUS_SUCCESS)
    return ZCL_STATUS_FAILURE;
  APP_ZB_DBG("Read back cluster information : SUCCESS");  

  UTIL_SEQ_SetTask(1U << CFG_TASK_LIGHT_UPDATE, CFG_SCH_PRIO_1);

  return ZCL_STATUS_SUCCESS;
} /* App_Light_Restore_State */


/* Identify & FindBind actions ----------------------------------------------------------*/
/**
 * @brief Put Identify cluster into identify mode to response at the binding queries emitted
 * 
 */
void App_Light_IdentifyMode(void)
{
  if (id_mode_on == true)
  {
    APP_ZB_DBG("WARNING: AN ID MODE IS ALREADY LAUNCHED on another EndPoint");
    return;
  }
  
  if (app_zb_info.join_status != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Error, device not on a network");
    return;
  }

  ZbZclIdentifyServerSetTime(app_Light_Control.identify_server, IDENTIFY_MODE_DELAY);
} /* App_Light_IdentifyMode */

/**
 * @brief Callback to manage easely the identify mode and the display
 * Needed to avoid all endpoints are at the same time in identify mode
 * 
 * @param cluster Identify pointer
 * @param state of the Identify mode
 * @param arg NULL in all cases
 */
static void App_Light_Identify_cb(struct ZbZclClusterT *cluster, enum ZbZclIdentifyServerStateT state, void *arg)
{
  char disp_identify[18];
  switch (state)
  {
    case ZCL_IDENTIFY_START :
      id_mode_on = true;
      APP_ZB_DBG("Turn on Identify Mode for %ds", IDENTIFY_MODE_DELAY);
      /* Display info on LCD */
      UTIL_LCD_ClearStringLine(DK_LCD_STATUS_LINE);
      sprintf(disp_identify, "Identify Mode %2ds", IDENTIFY_MODE_DELAY);
      UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_STATUS_LINE), (uint8_t *)disp_identify, CENTER_MODE);
      BSP_LCD_Refresh(0);
      break;

    case ZCL_IDENTIFY_STOP:
      id_mode_on = false;
      APP_ZB_DBG("Turn off Identify Mode");
      UTIL_SEQ_SetTask(1U << CFG_TASK_LCD_CLEAN_STATUS, CFG_SCH_PRIO_1);
      break;

    default :
      APP_ZB_DBG("Identification mode cb state unknown");
      break;    
  }
}; /* App_Light_Identify_cb */

/**
 * @brief  For debug purpose, display the local binding table information
 * 
 */
void App_Light_Bind_Disp (void)
{
  struct ZbApsmeBindT entry;

  printf("\n\r");
  APP_ZB_DBG("Binding Table");
  APP_ZB_DBG(" -----------------------------------------------------------------");
  APP_ZB_DBG(" Item |   Long Address   | ClusterId | Src Endpoint | Dst Endpoint");
  APP_ZB_DBG(" -----|------------------|-----------|--------------|-------------");

  /* Loop on the Binding Table */
  for (uint8_t i = 0;; i++)
  {
    if (ZbApsGetIndex(app_Light_Control.zb , ZB_APS_IB_ID_BINDING_TABLE, &entry, sizeof(entry), i) != ZB_APS_STATUS_SUCCESS)
    {
      break;
    }
    if (entry.srcExtAddr == 0ULL)
    {
      continue;
    }
    if (entry.clusterId != ZCL_CLUSTER_ONOFF)
    {
      continue;
    }    
    APP_ZB_DBG("  %2d  | %016llx |   0x%04x  |    0x%04x    |    0x%04x", i, entry.dst.extAddr, entry.clusterId, entry.srcEndpt, entry.dst.endpoint);
  }
  APP_ZB_DBG("-----------------------------------------------------------------\n\r");
} /* App_Light_Bind_Disp */


/* Light Device Application ------------------------------------------------- */
/**
 * @brief GPIO Init for Blinkt Bar LEDs
 * 
 */
static void App_Light_Blinkt_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /**GPIO Configuration
   * PB2     ------> clock
   * PD13    ------> data
  */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  BLINKT_Init(0, GPIOB, GPIO_PIN_2 , GPIOD, GPIO_PIN_13); 
} /* App_Light_Blinkt_GPIO_Init */

/**
 * @brief Toggle the light from menu using the cluster implemented
 * 
 */
void App_Light_Toggle(void)
{
  // Update the cluster server attribute and send report attribute automatically if exists one
  (void) light_onoff_server_toggle_cb(app_Light_Control.app_OnOff->onoff_server, NULL, NULL);
} /* App_Light_Toggle */

void App_Light_Level_Up(void)
{
  struct ZbZclLevelClientMoveReqT req = 
  {
    .mode = LIGHT_LEVEL_MODE_UP,
  };
  
  (void) light_level_server_move_cb(app_Light_Control.app_Level->level_server, &req, NULL, NULL);
} /* App_Light_Level_Up */

void App_Light_Level_Down(void)
{
  struct ZbZclLevelClientMoveReqT req = 
  {
    .mode = LIGHT_LEVEL_MODE_DOWN,
  };
  
  (void) light_level_server_move_cb(app_Light_Control.app_Level->level_server, &req, NULL, NULL);
} /* App_Light_Level_Down */

/**
 * @brief Refresh the Light status from the attribut of the clusters
 * 
 */
static void App_Light_Control_Task(void)
{
  if (app_Light_Control.app_OnOff->On)
  {
    HAL_Delay(10);
    BLINKT_SetLedLevel(0, 0xE0, app_Light_Control.app_Level->level,                                  0,                                  0);   //0b11100000
    BLINKT_SetLedLevel(0, 0x18, app_Light_Control.app_Level->level, app_Light_Control.app_Level->level, app_Light_Control.app_Level->level);   //0b00011000
    BLINKT_SetLedLevel(0, 0x07,                                  0,                                  0, app_Light_Control.app_Level->level);   //0b00000111
    APP_ZB_DBG("Light to ON");

    // Reuse LCD to display information
    UTIL_LCD_ClearStringLine(DK_LCD_LIGHT_DISP);
    UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_LIGHT_DISP), (uint8_t *) "LIGHT : ON", CENTER_MODE);
    BSP_LCD_Refresh(0);
  }
  else
  {
    HAL_Delay(10);
    BLINKT_SetOff(0);
    APP_ZB_DBG("Light to OFF");

    // Reuse LCD to display information
    UTIL_LCD_ClearStringLine(DK_LCD_LIGHT_DISP);
    UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_LIGHT_DISP), (uint8_t *) "LIGHT : OFF", CENTER_MODE);
    BSP_LCD_Refresh(0);  
  }
} /* App_Light_Control_Task */

