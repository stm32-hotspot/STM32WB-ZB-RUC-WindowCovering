/**
  ******************************************************************************
  * @file    app_roller_shutter_remote_window_covering.c
  * @author  Zigbee Application Team
  * @brief   Application interface for window covering cluster 
  *          of the Roller Shutter Remote Endpoint
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

/* External variables --------------------------------------------------------*/
extern Shutter_Remote_T app_Shutter_Remote_Control;

/* Application Variable-------------------------------------------------------*/
Window_Cov_Control_T app_Window_Covering_Control;

/* Window Covering callbacks Declaration -------------------------------------*/
static void App_Roller_Shutter_Remote_Window_Covering_ReportConfig_cb(struct ZbZclCommandRspT *cmd_rsp,void *arg);
static void App_Roller_Shutter_Remote_Window_Covering_Client_Report  (struct ZbZclClusterT *clusterPtr,
  struct ZbApsdeDataIndT *dataIndPtr, uint16_t attributeId, enum ZclDataTypeT dataType,
  const uint8_t *in_payload, uint16_t in_len);
static void App_Roller_Shutter_Remote_Window_Covering_Read_cb(const ZbZclReadRspT *readRsp, void *arg);
static void App_Roller_Shutter_Remote_Window_Covering_Cmd_cb (struct ZbZclCommandRspT * cmd_rsp, void *arg);
static char * Get_state_char(void);

/* Window Covering set/get cmd attribute ------------------------------------ */
void App_Roller_Shutter_Remote_Window_Covering_Set_Cmd(uint8_t cmd_send)
{
  app_Window_Covering_Control.cmd_send = cmd_send;
}

uint8_t App_Roller_Shutter_Remote_Window_Covering_Get_Cmd(void)
{
  return app_Window_Covering_Control.cmd_send;
}

void App_Roller_Shutter_Remote_Window_Covering_Set_state(uint8_t state)
{
  app_Window_Covering_Control.state = state;
}

uint8_t App_Roller_Shutter_Remote_Window_Covering_Get_state(void)
{
  return app_Window_Covering_Control.state;
}

/**
 * @brief Return the state of the action to display it
 * 
 * @return char* 
 */
static char * Get_state_char(void)
{
  switch (app_Window_Covering_Control.state)
  {
    case ZCL_WNCV_COMMAND_UP :
    return "Move Up";

    case ZCL_WNCV_COMMAND_STOP :
    return "Move Stop";

    case ZCL_WNCV_COMMAND_DOWN :
    return "Move Down";

    case ZCL_COMMAND_ADC_STOP :
    return "Pb Motor detection";
  
    default:
    return "Not recognize action";
  }
} /* Get_state_char */

// Clusters CFG ----------------------------------------------------------------
/**
 * @brief  Configure and register cluster for endpoint dedicated
 * 
 * @param  zb Zigbee stack instance
 * @retval Window_Cov_Control_T* endpoint control
 */
Window_Cov_Control_T * App_Roller_Shutter_Remote_Window_Covering_ConfigEndpoint(struct ZigBeeT *zb)
{
  /* Window Covering Client */
  app_Window_Covering_Control.window_covering_client = ZbZclWindowClientAlloc(zb, ROLLER_SHUTTER_REMOTE_ENDPOINT);    
  assert(app_Window_Covering_Control.window_covering_client != NULL);
  app_Window_Covering_Control.window_covering_client->report = &App_Roller_Shutter_Remote_Window_Covering_Client_Report;
  assert(ZbZclClusterEndpointRegister(app_Window_Covering_Control.window_covering_client) != NULL);

  /* default value for the Roller Shutter state */
  App_Roller_Shutter_Remote_Window_Covering_Set_state((uint8_t) ZCL_WNCV_COMMAND_STOP);

  return &app_Window_Covering_Control;
} /* App_Roller_Shutter_Remote_Window_Covering_ConfigEndpoint */

// -----------------------------------------------------------------------------
// Window Covering Attribute Report config -------------------------------------
/**
 * @brief  Configure the report for the Current Position attribute at each modification on the server
 * 
 * @param  dst address configuration endpoint
 * @retval None
 */
void App_Roller_Shutter_Remote_Window_Covering_ReportConfig(struct ZbApsAddrT * dst)
{
  enum   ZclStatusCodeT         status;
  struct ZbZclAttrReportConfigT reportCfg;

  /* Set Report Configuration  */
  memset( &reportCfg, 0, sizeof( reportCfg ) );
  
  reportCfg.dst = *dst; 
  reportCfg.num_records = 1;
  reportCfg.record_list[0].direction = ZCL_REPORT_DIRECTION_NORMAL;
  reportCfg.record_list[0].min       = ROLLER_SHUTTER_REMOTE_WINDOW_COVERING_MIN_REPORT;    
  reportCfg.record_list[0].max       = ROLLER_SHUTTER_REMOTE_WINDOW_COVERING_MAX_REPORT;
  reportCfg.record_list[0].change    = ROLLER_SHUTTER_REMOTE_WINDOW_COVERING_REPORT_CHANGE;
  reportCfg.record_list[0].attr_id   = ZCL_WNCV_SVR_ATTR_CURR_POS_LIFT_PERCENT;
  reportCfg.record_list[0].attr_type = ZCL_DATATYPE_UNSIGNED_8BIT;
  
  APP_ZB_DBG("Send Window Covering Report Config");
  status = ZbZclAttrReportConfigReq(app_Window_Covering_Control.window_covering_client, &reportCfg, &App_Roller_Shutter_Remote_Window_Covering_ReportConfig_cb, NULL);
  if ( status != ZCL_STATUS_SUCCESS )
  {
    APP_ZB_DBG("Error during Report Config Request 0x%02X", status );
  }
} /* App_Roller_Shutter_Remote_Window_Covering_ReportConfig */

/**
 * @brief  CallBack for the report configuration
 * @param  cmd_rsp response 
 * @param  arg unused
 * @retval None
 */
static void App_Roller_Shutter_Remote_Window_Covering_ReportConfig_cb(struct ZbZclCommandRspT * cmd_rsp, void *arg)
{
  /* retry_tab store all previous retry from all server. It's composed 
       of NB_OF_SERV_BINDABLE + 1 extra item in case of error */
  static Retry_number_T retry_tab[SIZE_RETRY_TAB] ;
  
  /* Retrieve the corresponding rety_nb with the specified extAddr */
  uint8_t * retry = Get_retry_nb (retry_tab, cmd_rsp->src.extAddr);
  
  /* Report failed, launch retry process */
  if ((cmd_rsp->status != ZCL_STATUS_SUCCESS) && (cmd_rsp->aps_status != ZB_STATUS_SUCCESS))
  {
    APP_ZB_DBG("Report Window Covering Config Failed error : 0x%016llx  | aps_status : 0x%02x | zcl_status : 0x%02x",cmd_rsp->src.extAddr, cmd_rsp->aps_status, cmd_rsp->status);    
    
    /* Check if max retry have already been reached */
    if ((*retry)++ < MAX_RETRY_REPORT )
    {
      APP_ZB_DBG("Retry %d | report Window Covering config 0x%016llx", (*retry), cmd_rsp->src.extAddr);
      /* Launch retry process */
      App_Roller_Shutter_Remote_Retry_Cmd (REPORT_CONF_WINDOW_ATTR, &(cmd_rsp->src));
      return;
    }
    else
    {
      /* Max retry reached */
      APP_ZB_DBG("Exceed max retry for ReportConfig cmd to 0x%016llx", cmd_rsp->src.extAddr);
    }
  }
  else
  {
    APP_ZB_DBG("Report Window Covering Config set with success");
    App_Roller_Shutter_Remote_Window_Covering_Read_Attribute( &(cmd_rsp->src) );
  }
  
  /* Reset retry counter */
  (*retry) = 0;  
} /* App_Roller_Shutter_Remote_Window_Covering_ReportConfig_cb */

/**
 * @brief  Report the modification of ZCL_WNCV_SVR_ATTR_CURR_POS_LIFT_PERCENT and update locally the state
 * @param  clusterPtr
 * @retval None
 */
static void App_Roller_Shutter_Remote_Window_Covering_Client_Report(struct ZbZclClusterT *clusterPtr,
  struct ZbApsdeDataIndT *dataIndPtr, uint16_t attributeId, enum ZclDataTypeT dataType,
  const uint8_t *in_payload, uint16_t in_len)
{
  int attrLen;

  /* default value for the Roller Shutter state */
  uint8_t state = (uint8_t) ZCL_WNCV_COMMAND_STOP; 

  /* Attribute reporting */
  if (attributeId == ZCL_WNCV_SVR_ATTR_CURR_POS_LIFT_PERCENT)
  {
    attrLen = ZbZclAttrParseLength(dataType, in_payload, dataIndPtr->asduLength, 0);
    if (attrLen < 0)
    {
      APP_ZB_DBG("Report error length 0");
      return;
    }
    if (attrLen > (int)in_len)
    {
      APP_ZB_DBG("Report error length >");
      return;
    }
    if (dataIndPtr->dst.endpoint != ROLLER_SHUTTER_REMOTE_ENDPOINT)
    {
      APP_ZB_DBG("Report error wrong endpoint (%d)", dataIndPtr->dst.endpoint);
      return;
    }

    state = (uint8_t) in_payload[0];
    App_Roller_Shutter_Remote_Window_Covering_Set_state(state);
    switch (state)
    {
      case ZCL_WNCV_COMMAND_UP :
      case ZCL_WNCV_COMMAND_STOP :
      case ZCL_WNCV_COMMAND_DOWN :
      case ZCL_COMMAND_ADC_STOP :
        App_Roller_Shutter_Remote_Led_Blink();
        break;
      default :
        APP_ZB_DBG("Unknown code : %x", state);
        break;
    }

    APP_ZB_DBG("Report attribute From %016llx  -  %s", dataIndPtr->src.extAddr, Get_state_char());
  }
} /* App_Roller_Shutter_Remote_Window_Covering_client_report */

/* Window Covering Read Attribute  --------------------------------------------------- */
/**
 * @brief Read OTA Attribute to update the local status
 * @param  target addresse
 * @retval None
 */
void App_Roller_Shutter_Remote_Window_Covering_Read_Attribute(struct ZbApsAddrT * dst)
{
  ZbZclReadReqT readReq;
  uint64_t epid = 0U;
  enum ZclStatusCodeT rd_status;

  /* Check that the Zigbee stack initialised */
  if(app_Window_Covering_Control.window_covering_client->zb == NULL)
  {
    return;
  }  
  /* Check if the device joined the network */
  if (ZbNwkGet(app_Window_Covering_Control.window_covering_client->zb, ZB_NWK_NIB_ID_ExtendedPanId, &epid, sizeof(epid)) != ZB_STATUS_SUCCESS)
  {
    return;
  }
  if (epid == 0U)
  {
    return;
  }

  /* Create the read request for the attribut */
  memset(&readReq, 0, sizeof(readReq));
  // readReq.dst     = app_Window_Covering_Control.bind_table[0];
  readReq.dst     = *dst;
  readReq.count   = 1U;
  readReq.attr[0] = ZCL_WNCV_SVR_ATTR_CURR_POS_LIFT_PERCENT ;
   
  APP_ZB_DBG("Read the Window Covering Attribute");
  rd_status = ZbZclReadReq(app_Window_Covering_Control.window_covering_client, &readReq, App_Roller_Shutter_Remote_Window_Covering_Read_cb, dst);
  if ( rd_status != ZCL_STATUS_SUCCESS )
  {
    APP_ZB_DBG("Error during Window Covering read request status : 0x%02X", rd_status );
  }
} /* App_Roller_Shutter_Remote_Window_Covering_Read_Attribute */

/**
 * @brief  Read OTA Window Covering attribute callback
 * @param  read rsp
 * @retval None
 */
static void App_Roller_Shutter_Remote_Window_Covering_Read_cb(const ZbZclReadRspT * cmd_rsp, void * arg)
{
  /* retry_tab store all previous retry from all server. It's composed 
       of NB_OF_SERV_BINDABLE + 1 extra item in case of error */
  static Retry_number_T retry_tab[SIZE_RETRY_TAB] ;
  
  /* Retrieve the corresponding rety_nb with the specified extAddr */
  uint8_t * retry = Get_retry_nb (retry_tab, cmd_rsp->src.extAddr);
  
  /* default value for the Roller Shutter state */
  uint8_t state = (uint8_t) ZCL_WNCV_COMMAND_STOP; 

  /* Read failed, launch retry process */
  if (cmd_rsp->status != ZCL_STATUS_SUCCESS)
  {   
    APP_ZB_DBG("Error, Read cmd failed | status : 0x%x", cmd_rsp->status);
    /* Check if max retry have already been reached */
    if ( (*retry)++ < MAX_RETRY_REPORT )
    {    
      APP_ZB_DBG("Retry %d | read Window Covering 0x%016llx", (*retry), cmd_rsp->src.extAddr);
      /* Launch retry process */
      App_Roller_Shutter_Remote_Retry_Cmd (READ_WINDOW_ATTR,  &(cmd_rsp->src));
      return;    
    }
    else
    {
      /* Max retry reached */
      APP_ZB_DBG("Exceed max retry for Read cmd to 0x%016llx", cmd_rsp->src.extAddr);
    }    
  }
  
  state = *(cmd_rsp->attr[0].value);
  App_Roller_Shutter_Remote_Window_Covering_Set_state(state);
  APP_ZB_DBG("Read attribute From %016llx  -  %s", cmd_rsp->src.extAddr, Get_state_char());

  /* Reset retry counter */
  (*retry) = 0;
} /* App_Roller_Shutter_Remote_Window_Covering_Read_cb */


/* Window Covering send cmd ------------------------------------------------- */
/**
 * @brief  Window Covering pushed command req send
 * @param  target, if NULL send cmd to all server bind
 * @retval None
 */
void App_Roller_Shutter_Remote_Window_Covering_Cmd(struct ZbApsAddrT * dst)
{
  uint64_t epid = 0U;
  enum ZclStatusCodeT cmd_status;
  
  /* Check that the Zigbee stack initialised */
  if(app_Window_Covering_Control.window_covering_client->zb == NULL)
  {
    return;
  }
  
  /* Check if the device joined the network */
  if (ZbNwkGet(app_Window_Covering_Control.window_covering_client->zb, ZB_NWK_NIB_ID_ExtendedPanId, &epid, sizeof(epid)) != ZB_STATUS_SUCCESS)
  {
    return;
  }

  if (epid == 0U)
  {
    return;
  }
  
  /* No target specified, send cmd to all server binded */
  if ( dst == NULL )
  {
    /* First check if the semaphore is available */
    if ( app_Shutter_Remote_Control.is_rdy_for_next_cmd == 0 )
    {
      /* Browse binding table to find target */
      for (uint8_t i = 0; i < app_Window_Covering_Control.bind_nb; i++)
      {
        /* Select command to send at each server */
        switch (app_Window_Covering_Control.cmd_send)
        {
          case ZCL_WNCV_COMMAND_UP :
            cmd_status = ZbZclWindowClientCommandUp(app_Window_Covering_Control.window_covering_client, &app_Window_Covering_Control.bind_table[i] , &App_Roller_Shutter_Remote_Window_Covering_Cmd_cb, NULL);
            break;
          case ZCL_WNCV_COMMAND_DOWN :
            cmd_status = ZbZclWindowClientCommandDown(app_Window_Covering_Control.window_covering_client, &app_Window_Covering_Control.bind_table[i] , &App_Roller_Shutter_Remote_Window_Covering_Cmd_cb, NULL);
            break;
          case ZCL_WNCV_COMMAND_STOP :
            cmd_status = ZbZclWindowClientCommandStop(app_Window_Covering_Control.window_covering_client, &app_Window_Covering_Control.bind_table[i] , &App_Roller_Shutter_Remote_Window_Covering_Cmd_cb, NULL);
            break;        
        }
       
        app_Window_Covering_Control.is_init = true;
        
        /* Take the semaphore */
        app_Shutter_Remote_Control.is_rdy_for_next_cmd ++;      
      }
    }
    else 
    {
      APP_ZB_DBG("W8 for all acknwoledge");
    }
  }
  else
  {
    /* Select command to send at the server selected */
    switch (app_Window_Covering_Control.cmd_send)
    {
      case ZCL_WNCV_COMMAND_UP :
        cmd_status = ZbZclWindowClientCommandUp(app_Window_Covering_Control.window_covering_client, dst , &App_Roller_Shutter_Remote_Window_Covering_Cmd_cb, NULL);
        break;
      case ZCL_WNCV_COMMAND_DOWN :
        cmd_status = ZbZclWindowClientCommandDown(app_Window_Covering_Control.window_covering_client, dst , &App_Roller_Shutter_Remote_Window_Covering_Cmd_cb, NULL);
        break;
      case ZCL_WNCV_COMMAND_STOP :
        cmd_status = ZbZclWindowClientCommandStop(app_Window_Covering_Control.window_covering_client, dst , &App_Roller_Shutter_Remote_Window_Covering_Cmd_cb, NULL);
        break;        
    }      
  }

  /* check status of command request send to the Server */
  if (cmd_status != ZCL_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Error, ZbZclWindowClientCommand failed : 0x%x", cmd_status);
    app_Shutter_Remote_Control.is_rdy_for_next_cmd --;
  }
} /* App_Roller_Shutter_Remote_Window_Covering_Cmd */

/**
 * @brief  CallBack for the window cmd
 * @param  command response 
 * @param  arg passed trough cb function
 * @retval None
 */
static void App_Roller_Shutter_Remote_Window_Covering_Cmd_cb (struct ZbZclCommandRspT * cmd_rsp, void *arg)
{
  /* retry_tab store all previous retry from all server. It's composed 
       of NB_OF_SERV_BINDABLE + 1 extra item in case of error */
  static Retry_number_T retry_tab[SIZE_RETRY_TAB] ;
  
  /* Retrieve the corresponding rety_nb with the specified extAddr */
  uint8_t * retry = Get_retry_nb (retry_tab, cmd_rsp->src.extAddr);

  if ((cmd_rsp->aps_status != ZB_STATUS_SUCCESS) && (cmd_rsp->status != ZCL_STATUS_SUCCESS))
  {
    APP_ZB_DBG("client  0x%016llx didn't responded window cmd | aps_status : 0x%02x | zcl_status : 0x%02x",cmd_rsp->src.extAddr, cmd_rsp->aps_status, cmd_rsp->status);
    if ( (*retry)++ < MAX_RETRY_CMD )
    {
      APP_ZB_DBG("Retry %d | Window cmd to 0x%016llx", (*retry) ,cmd_rsp->src.extAddr);
      App_Roller_Shutter_Remote_Retry_Cmd (WRITE_WINDOW_ATTR, &(cmd_rsp->src));
      return;
    }
    else
    {
      APP_ZB_DBG("Exceed max retry for sending Window cmd to 0x%016llx", cmd_rsp->src.extAddr);
    }
  }
  else
  {
    APP_ZB_DBG("Response cb from Window cmd from client :  0x%016llx ", cmd_rsp->src.extAddr);
  }

  
  /* Reset retry number */
   (*retry) = 0;
  
  /* Release the semaphore */
  app_Shutter_Remote_Control.is_rdy_for_next_cmd --;

} /* App_Roller_Shutter_Remote_Window_Covering_Cmd_cb */

