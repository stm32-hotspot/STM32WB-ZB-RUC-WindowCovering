/**
  ******************************************************************************
  * @file    app_roller_shutter_remote.c
  * @author  Zigbee Application Team
  * @brief   Application interface of Window controller Endpoint
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
#include "app_roller_shutter_remote_cfg.h"

/* Private Define-------------------------------------------------------------*/
#define TS_LED_BLINK_DELAY       (200 * HW_TS_SERVER_1ms_NB_TICKS)

/* Private Variables----------------------------------------------------------*/
static uint8_t       TS_ID_LED_BLINK;

/* Application Variable-------------------------------------------------------*/
Shutter_Remote_T app_Shutter_Remote_Control =
{
  .is_rdy_for_next_cmd = 0U,
  .force_read          = false,
};

/* Finding&Binding function Declaration --------------------------------------*/
static void App_Roller_Shutter_Remote_FindBind_cb(enum ZbStatusCodeT status, void *arg);
static void App_Roller_Shutter_Remote_Bind_Nb    (void);

/* Retry function Declaration ------------------------------------------------*/
static void App_Roller_Shutter_Remote_Task_Retry_Cmd(void);

/* App Window Covering functions ---------------------------------------------*/
static void App_Roller_Shutter_Remote_Status_Led (void);


// Clusters CFG ----------------------------------------------------------------
/**
 * @brief  Configure and register Zigbee application Window Controller endpoint, with corresponding clusters
 * composing the endpoint
 * 
 * @param  zb Zigbee stack instance
 * @retval None
 */
void App_Roller_Shutter_Remote_ConfigEndpoint(struct ZigBeeT *zb)
{
  struct ZbApsmeAddEndpointReqT   req;
  struct ZbApsmeAddEndpointConfT  conf;
  
  /* Endpoint: WINDOW_ENDPOINT */
  memset(&req, 0, sizeof(req));
  req.profileId = ZCL_PROFILE_HOME_AUTOMATION;
  req.deviceId  = ZCL_DEVICE_WINDOW_COVERING_CONTROLLER;
  req.endpoint  = ROLLER_SHUTTER_REMOTE_ENDPOINT;
  ZbZclAddEndpoint(zb, &req, &conf);
  assert(conf.status == ZB_STATUS_SUCCESS);

  /* Idenfity client */
  app_Shutter_Remote_Control.identify_client = ZbZclIdentifyClientAlloc(zb, ROLLER_SHUTTER_REMOTE_ENDPOINT);
  assert(app_Shutter_Remote_Control.identify_client != NULL);
  assert(ZbZclClusterEndpointRegister(app_Shutter_Remote_Control.identify_client)!= NULL);

  /* Window Covering Client */
  app_Shutter_Remote_Control.app_Window_Covering_Control = App_Roller_Shutter_Remote_Window_Covering_ConfigEndpoint(zb);
  
  /* make a local pointer of Zigbee stack to read easier */
  app_Shutter_Remote_Control.zb = zb;

  /* Init retry process */
  UTIL_SEQ_RegTask(1U << CFG_TASK_RETRY_PROC, UTIL_SEQ_RFU, App_Roller_Shutter_Remote_Task_Retry_Cmd);
  app_Shutter_Remote_Control.cmd_retry.dst = (struct ZbApsAddrT *) malloc( sizeof(struct ZbApsAddrT) );

  /* Command status on LEDs */
  UTIL_SEQ_RegTask(1U << CFG_TASK_LED_BLINK, UTIL_SEQ_RFU, App_Roller_Shutter_Remote_Status_Led);
  HW_TS_Create(CFG_TIM_LED_BLINK, &TS_ID_LED_BLINK, hw_ts_Repeated, App_Roller_Shutter_Remote_Status_Led);
} /* App_Roller_Shutter_Remote_ConfigEndpoint */

/**
 * @brief  Associate the Endpoint to the group address (if needed)
 * 
 * @param  None
 * @retval None
 */
void App_Roller_Shutter_Remote_ConfigGroupAddr(void)
{
  struct ZbApsmeAddGroupReqT  req;
  struct ZbApsmeAddGroupConfT conf;
  
  memset(&req, 0, sizeof(req));
  req.endpt     = ROLLER_SHUTTER_REMOTE_ENDPOINT;
  req.groupAddr = ROLLER_SHUTTER_REMOTE_GROUP_ADDR;
  ZbApsmeAddGroupReq(app_Shutter_Remote_Control.zb, &req, &conf);
} /* App_Roller_Shutter_Remote_ConfigGroupAddr */

/**
 * @brief  Restore the state after persitance data loaded
 * 
 * @param  None
 * @retval None
 */
void App_Roller_Shutter_Remote_Restore_State(void)
{
  struct ZbApsmeBindT entry;

  // Recalculate the effective binding
  App_Roller_Shutter_Remote_Bind_Nb();
  APP_ZB_DBG("Restore state %2d bind", app_Shutter_Remote_Control.app_Window_Covering_Control->bind_nb);

  /* Browse binding table to retrieve attribute value */
  for (uint8_t i = 0; ; i++)
  {
    if (ZbApsGetIndex(app_Shutter_Remote_Control.zb, ZB_APS_IB_ID_BINDING_TABLE, &entry, sizeof(entry), i) != ZB_APS_STATUS_SUCCESS)
    {
      break;
    }
    if (entry.srcExtAddr == 0ULL)
    {
      continue;
    }

    // Reread the attribute from server and synchronize the differents endpoint if needed
    switch (entry.clusterId) 
    {
      case ZCL_CLUSTER_WINDOW_COVERING :
        App_Roller_Shutter_Remote_Window_Covering_Read_Attribute( &entry.dst );
        break;
        
      default :
        // APP_ZB_DBG("Try to restore unknown cluster ID : %d",entry.clusterId);
        break;
    } 
  }
} /* App_Roller_Shutter_Remote_Restore_State */


// FindBind actions ------------------------------------------------------------
/**
 * @brief  Start Finding and Binding process as an initiator.
 * Call App_Roller_Shutter_Remote_FindBind_cb when successfull to configure correctly the binding
 * 
 * @param  None
 * @retval None
 */
void App_Roller_Shutter_Remote_FindBind(void)
{
  uint8_t status = ZCL_STATUS_FAILURE;
  uint64_t epid = 0U;

  if (app_Shutter_Remote_Control.zb == NULL)
  {
    APP_ZB_DBG("Error, zigbee stack not initialized");
    return;
  }

  /* Check if the router joined the network */
  if (ZbNwkGet(app_Shutter_Remote_Control.zb, ZB_NWK_NIB_ID_ExtendedPanId, &epid, sizeof(epid)) != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Error, failed to get network information");
    return;
  }
  if (epid == 0U)
  {
    APP_ZB_DBG("Error, device not on a network");
    return;
  }
  
  APP_ZB_DBG("Initiate F&B");
  status = ZbStartupFindBindStart(app_Shutter_Remote_Control.zb, &App_Roller_Shutter_Remote_FindBind_cb, NULL);

  if (status != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG(" Error, cannot start Finding & Binding, status = 0x%02x", status);
  }
} /* App_Roller_Shutter_Remote_FindBind */

/**
 * @brief  Task called after F&B process to configure a report and update the local status.
 *         Will call the configuration report for the good clusterId.
 *         Will turn on Green LED if bindings are successfully created.
 * @param  None
 * @retval None
 */
static void App_Roller_Shutter_Remote_FindBind_cb(enum ZbStatusCodeT status, void *arg)
{
  static uint8_t retry = 0;

  if (status != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Error while F&B | error code : 0x%02X", status);
    // Retry command if failed
    if (retry++ < MAX_RETRY_REPORT )
    {
      APP_ZB_DBG("Retry %d", retry);
      App_Roller_Shutter_Remote_Retry_Cmd (FIND_AND_BIND, NULL);
      return;
    }
  }
  else
  {
    struct ZbApsmeBindT entry;
  
    APP_ZB_DBG(" Item |   ClusterId | Long Address     | End Point");
    APP_ZB_DBG(" -----|-------------|------------------|----------");

    /* Loop only on the new binding element */
    for (uint8_t i = app_Shutter_Remote_Control.app_Window_Covering_Control->bind_nb ;; i++)
    {
      if (ZbApsGetIndex(app_Shutter_Remote_Control.zb, ZB_APS_IB_ID_BINDING_TABLE, &entry, sizeof(entry), i) != ZB_APS_STATUS_SUCCESS)
      {
        break;
      }
      if (entry.srcExtAddr == 0ULL)
      {
        continue;
      }
    
      /* display binding infos */
      APP_ZB_DBG("  %2d  |     0x%03x   | %016llx |   %2d", i, entry.clusterId, entry.dst.extAddr, entry.dst.endpoint);
        
      // Report on the Cluster selected to know when a modification status
      switch (entry.clusterId)
      {
        case ZCL_CLUSTER_WINDOW_COVERING :
          // adding a new binding item locally for My_Cluster cluster
          memcpy(&app_Shutter_Remote_Control.app_Window_Covering_Control->bind_table[app_Shutter_Remote_Control.app_Window_Covering_Control->bind_nb++], &entry.dst, sizeof(struct ZbApsAddrT));
          // start report config proc
          App_Roller_Shutter_Remote_Window_Covering_ReportConfig( &entry.dst);
          break;

        case ZCL_CLUSTER_IDENTIFY :
          break;
          
        default:
          // APP_ZB_DBG("F&B WRONG CLUSTER");          
          break;
      }
    }
  
  APP_ZB_DBG("Binding entries created: %d", app_Shutter_Remote_Control.app_Window_Covering_Control->bind_nb);
  }
  /* Reset retry value */
  retry = 0;
} /* App_Roller_Shutter_Remote_FindBind_cb */

/**
 * @brief  Checks the number of valid entries in the local binding table and update the local attribut.
 * Filters it by clusterId, so if multiple clusters only the local clusters is taken
 * @param  None
 * @retval None
 */
static void App_Roller_Shutter_Remote_Bind_Nb(void)
{
  struct ZbApsmeBindT entry;
  
  app_Shutter_Remote_Control.app_Window_Covering_Control->bind_nb = 0;
  
   memset(&app_Shutter_Remote_Control.app_Window_Covering_Control->bind_table, 0, sizeof(uint64_t) * NB_OF_SERV_BINDABLE);

  /* Loop on the local binding table */
  for (uint8_t i = 0;; i++)
  {
    /* check the end of the table */
    if (ZbApsGetIndex(app_Shutter_Remote_Control.zb, ZB_APS_IB_ID_BINDING_TABLE, &entry, sizeof(entry), i) != ZB_APS_STATUS_SUCCESS)
    {
      break;
    }
    /* remove itself */
    if (entry.srcExtAddr == 0ULL)
    {
      continue;
    }

    /* do not reference useless cluster */
    switch (entry.clusterId)
    {
      case ZCL_CLUSTER_WINDOW_COVERING :
        /* locally increment bind number of selected cluster and add extended addr to corresponding local binding table */
        memcpy(&app_Shutter_Remote_Control.app_Window_Covering_Control->bind_table[app_Shutter_Remote_Control.app_Window_Covering_Control->bind_nb++], &entry.dst, sizeof(struct ZbApsAddrT));        
        break;

      default:
        continue;
        break;
    }    
  }

} /* App_Roller_Shutter_Remote_Bind_Nb */

/**
 * @brief  For debug purpose, display the local binding table information
 * @param  None 
 * @retval None
 */
void App_Roller_Shutter_Remote_Bind_Disp(void)
{
  APP_ZB_DBG("Binding Table has %d items", app_Shutter_Remote_Control.app_Window_Covering_Control->bind_nb);
    
  /* only if the binding Table not empty */
  if (app_Shutter_Remote_Control.app_Window_Covering_Control->bind_nb > 0)
  {
    APP_ZB_DBG(" --------------------------------------------------");
    APP_ZB_DBG(" Item |   Long Address   | ClusterId | Dst Endpoint");
    APP_ZB_DBG(" -----|------------------|-----------|-------------");

    /* Loop on the Binding Table */
    for (uint8_t i = 0;i < app_Shutter_Remote_Control.app_Window_Covering_Control->bind_nb; i++)
    {
      APP_ZB_DBG("  %2d  | %016llx |   Window  |  0x%04x", i, app_Shutter_Remote_Control.app_Window_Covering_Control->bind_table[i].extAddr, app_Shutter_Remote_Control.app_Window_Covering_Control->bind_table[i].endpoint);
    }  
    APP_ZB_DBG(" --------------------------------------------------\n\r");
  } 
} /* App_Roller_Shutter_Remote_Bind_Disp */


// Retry function --------------------------------------------------------------
/**
 * @brief  Relaunch previously failed cmd
 * @param  None 
 * @retval None
 */
static void App_Roller_Shutter_Remote_Task_Retry_Cmd (void)
{     
  switch (app_Shutter_Remote_Control.cmd_retry.cmd_type)
  {
    case FIND_AND_BIND :
      App_Roller_Shutter_Remote_FindBind();
      break;        
    case REPORT_CONF_WINDOW_ATTR :
      App_Roller_Shutter_Remote_Window_Covering_ReportConfig( app_Shutter_Remote_Control.cmd_retry.dst );
      break;  
    case READ_WINDOW_ATTR :
      App_Roller_Shutter_Remote_Window_Covering_Read_Attribute( app_Shutter_Remote_Control.cmd_retry.dst );
      break;
    case WRITE_WINDOW_ATTR :
      App_Roller_Shutter_Remote_Window_Covering_Cmd( app_Shutter_Remote_Control.cmd_retry.dst );
      break; 
    default :
      APP_ZB_DBG("The command :%d haven't been implemented yet",app_Shutter_Remote_Control.cmd_retry.cmd_type);
      break;
  }

  app_Shutter_Remote_Control.cmd_retry.cmd_type = IDLE;
  return;
} /* App_Roller_Shutter_Remote_Task_Retry_Cmd */

/**
 * @brief  Add retry task to avoid recursivity
 * @param  Command type 
 * @param  Target address
 * @retval None
 */
void App_Roller_Shutter_Remote_Retry_Cmd (Cmd_Type_T cmd, const struct ZbApsAddrT * dst)
{
  /* Check if last command as been executed */
  if (app_Shutter_Remote_Control.cmd_retry.cmd_type == IDLE)
  {
    app_Shutter_Remote_Control.cmd_retry.cmd_type = cmd;
    memcpy(app_Shutter_Remote_Control.cmd_retry.dst, dst,sizeof(struct ZbApsAddrT ));
    
    UTIL_SEQ_SetTask(1U << CFG_TASK_RETRY_PROC, CFG_SCH_PRIO_0);    
  }
  else
  {
    APP_ZB_DBG("Trying to add %x cmd while already one hasen't been cleared : %x", cmd, app_Shutter_Remote_Control.cmd_retry.cmd_type);
  }
} /* App_Roller_Shutter_Remote_Retry_Cmd */

/**
 * @brief  Get the retry number of a command for a dedicated server
 * @param  None 
 * @retval retry_nb corresponding to server
 */
uint8_t * Get_retry_nb (Retry_number_T retry_tab[SIZE_RETRY_TAB], uint64_t server_addr)
{       
  
  /* Check in retry_tab if this serv has already failed this cmd */
  for (uint8_t i = 0 ; i < NB_OF_SERV_BINDABLE; i++)
  {
    if ( (server_addr == retry_tab[i].ext_addr) || (retry_tab[i].ext_addr == NULL))
    {
      return &retry_tab[i].retry_nb;
    }
  }
  
  APP_ZB_DBG("Didn't find the corresponding binded server");
  return &retry_tab[RETRY_ERROR_INDEX].retry_nb;
} /* Get_retry_nb */

/* Window control device -----------------------------------------------------*/
/**
 * @brief Wrapper function to control locally the roller shutter
 * Calling by Menu
 * 
 */
void App_Roller_Shutter_Remote_Move_Up  (void)
{
  /* To avoid a concurrent command by the retry process */
  if (App_Roller_Shutter_Remote_Window_Covering_Get_state() != ZCL_WNCV_COMMAND_UP)
  {
    App_Roller_Shutter_Remote_Window_Covering_Set_Cmd(ZCL_WNCV_COMMAND_UP);
    App_Roller_Shutter_Remote_Window_Covering_Cmd(NULL);  
  }
} /* App_Roller_Shutter_Remote_Move_Up */

void App_Roller_Shutter_Remote_Move_Down(void)
{
  /* To avoid a concurrent command by the retry process */
  if (App_Roller_Shutter_Remote_Window_Covering_Get_state() != ZCL_WNCV_COMMAND_DOWN)
  {
    App_Roller_Shutter_Remote_Window_Covering_Set_Cmd(ZCL_WNCV_COMMAND_DOWN);
    App_Roller_Shutter_Remote_Window_Covering_Cmd(NULL);  
  }
} /* App_Roller_Shutter_Remote_move_Down */

void App_Roller_Shutter_Remote_Move_Stop(void)
{
  App_Roller_Shutter_Remote_Window_Covering_Set_Cmd(ZCL_WNCV_COMMAND_STOP);
  App_Roller_Shutter_Remote_Window_Covering_Cmd(NULL);  
} /* App_Roller_Shutter_Remote_Move_Stop */


/* App function -------------------------------------------------------------*/
/**
 * @brief Set the LEDs according to the current command
 * 
 */
static void App_Roller_Shutter_Remote_Status_Led(void)
{  
  switch (App_Roller_Shutter_Remote_Window_Covering_Get_state())
  {
    case ZCL_WNCV_COMMAND_UP :
      BSP_LED_Toggle(LED1);
      BSP_LED_Off   (LED2);
      BSP_LED_Off   (LED3);
      break;
    case ZCL_WNCV_COMMAND_STOP :
      BSP_LED_Off(LED1);
      BSP_LED_Off(LED2);
      BSP_LED_Off(LED3);
      HW_TS_Stop(TS_ID_LED_BLINK);
      break;
    case ZCL_WNCV_COMMAND_DOWN :
      BSP_LED_Off   (LED1);
      BSP_LED_Off   (LED2);
      BSP_LED_Toggle(LED3);
      break;      
    case ZCL_COMMAND_ADC_STOP :
      BSP_LED_Toggle(LED1); 
      BSP_LED_Toggle(LED2);
      BSP_LED_Toggle(LED3);
      break;
    default :
      APP_ZB_DBG("Unknown code : %x",app_Shutter_Remote_Control.app_Window_Covering_Control->state);
      break;
  }
}; /* App_Roller_Shutter_Remote_Status_Led */

/**
 * @brief Launch the status led in parallel task
 * 
 */
void App_Roller_Shutter_Remote_Led_Blink(void)
{
  HW_TS_Start(TS_ID_LED_BLINK, TS_LED_BLINK_DELAY);
} /* App_Roller_Shutter_Remote_Led_Blink */


