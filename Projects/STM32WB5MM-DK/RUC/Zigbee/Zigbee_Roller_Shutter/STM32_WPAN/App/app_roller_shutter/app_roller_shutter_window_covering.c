/**
  ******************************************************************************
  * @file    app_roller_shutter_window_covering.c
  * @author  Zigbee Application Team
  * @brief   Application interface for Window covering cluster functionnality of the Roller shutter Endpoint
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
#include "app_roller_shutter_cfg.h"
#include "stm32_seq.h"

/* Application Variable-------------------------------------------------------*/
Window_Cov_Control_T app_Window_Cov_Control =
{
  .window_cmd   = ZCL_WNCV_COMMAND_STOP,
};

/* Application Variable----------------------------------------------------- */
/* Window  Attributes persistent flag set */


/* Clusters CFG ------------------------------------------------------------ */
/**
 * @brief  Configure and register Zigbee application endpoints, window covering callbacks
 * @param  zb Zigbee stack instance
 * @retval None
 */
Window_Cov_Control_T * App_Roller_Shutter_Window_Covering_Config(struct ZigBeeT *zb)
{
  struct ZbZclWindowServerCallbacksT WindowServerCb;
  
  /* callbacks Window */
  memset(&WindowServerCb, 0, sizeof(WindowServerCb));
  WindowServerCb.up_command   = Window_Server_Up_Cb;
  WindowServerCb.down_command = Window_Server_Down_Cb;
  WindowServerCb.stop_command = Window_Server_Stop_Cb;
  
  /* Window Server */
  app_Window_Cov_Control.window_server = ZbZclWindowServerAlloc(zb, ROLLER_SHUTTER_ENDPOINT,&WindowServerCb,NULL);
  assert(app_Window_Cov_Control.window_server != NULL);
  if (ZbZclClusterEndpointRegister(app_Window_Cov_Control.window_server) == false)
  {
    APP_ZB_DBG("Error while Registering window server");
  }

  if (ZbZclWindowClosureServerMode(app_Window_Cov_Control.window_server, ZCL_WNCV_STATUS_OPERATIONAL) != ZCL_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Config failed");
  }

  return &app_Window_Cov_Control;
} /* App_Roller_Shutter_Window_Covering_Config */

/**
 * @brief  Restore the state at startup from persistence
 * @param  None
 * @retval stack status code
 */
enum ZclStatusCodeT App_Roller_Shutter_Window_Covering_Restore_State(void)
{
  enum ZclStatusCodeT status = ZCL_STATUS_FAILURE;
  //TODO Double check for both attributs
  status = ZbZclAttrRead(app_Window_Cov_Control.window_server, 
                        ZCL_WNCV_SVR_ATTR_CURR_POS_LIFT_PERCENT, NULL,
                        &(app_Window_Cov_Control.window_cmd), sizeof(app_Window_Cov_Control.window_cmd), false);
  status = ZbZclAttrRead(app_Window_Cov_Control.window_server, 
                        ZCL_WNCV_SVR_ATTR_CURR_POS_TILT_PERCENT, NULL,
                        &(app_Window_Cov_Control.window_cmd), sizeof(app_Window_Cov_Control.window_cmd), false);

  /* Locally manage error if possible */
  switch(status)
  {
    case ZCL_STATUS_UNSUPP_ATTRIBUTE :
    case ZCL_STATUS_UNREPORTABLE_ATTRIBUTE :
    case ZCL_STATUS_INSUFFICIENT_SPACE :
    case ZCL_STATUS_FAILURE :
      APP_ZB_DBG("Read back cluster information : FAILED");
      status = ZCL_STATUS_FAILURE;
      break;
    default :
      status = ZCL_STATUS_SUCCESS;
      break;
  }
 
  return status;
} /* App_Roller_Shutter_Window_Covering_Restore_State */


/**
 * @brief Get the current command
 * 
 * @return uint8_t current command
 */
uint8_t App_Roller_Shutter_Window_Covering_Get_Cmd(void)
{
  return app_Window_Cov_Control.window_cmd;
} /* App_Roller_Shutter_Window_Covering_Get_Cmd */

/**
 * @brief Set the current command
 * 
 * @param window_cmd
 */
void App_Roller_Shutter_Window_Covering_Set_Cmd(uint8_t window_cmd)
{
  app_Window_Cov_Control.window_cmd = window_cmd;
} /* App_Roller_Shutter_Window_Covering_Set_Cmd */


/* Window callbacks Definition ---------------------------------------------- */
/**
 * @brief  Window server Up command callback
 * @param  cluster pointer to cluster server
 * @param  zclHdrPtr ZigBee header
 * @param  dataIndPtr source addr
 * @param  arg extra arg
 * @retval stack status code
 */
enum ZclStatusCodeT Window_Server_Up_Cb
(struct ZbZclClusterT *cluster, struct ZbZclHeaderT *zclHdrPtr,
 struct ZbApsdeDataIndT *dataIndPtr, void *arg)
{
  APP_ZB_DBG("Up Command : Coming From : 0x%x ",dataIndPtr->src.nwkAddr);
  
  // Check if a different command run to take it the new one and launch the motor control
  if (App_Roller_Shutter_Window_Covering_Get_Cmd() != ZCL_WNCV_COMMAND_UP)
  {
    if ((ZbZclAttrIntegerWrite(cluster, ZCL_WNCV_SVR_ATTR_CURR_POS_LIFT_PERCENT, ZCL_WNCV_COMMAND_UP) != ZCL_STATUS_SUCCESS) ||
        (ZbZclAttrIntegerWrite(cluster, ZCL_WNCV_SVR_ATTR_CURR_POS_TILT_PERCENT, ZCL_WNCV_COMMAND_UP) != ZCL_STATUS_SUCCESS))
    { 
      APP_ZB_DBG("Error ZbZclAttrIntegerWrite");
      return ZCL_STATUS_FAILURE;
    }

    App_Roller_Shutter_Window_Covering_Set_Cmd((uint8_t) ZCL_WNCV_COMMAND_UP);
    UTIL_SEQ_SetTask(1U << CFG_TASK_MOTOR_CONTROL, UTIL_SEQ_RFU);
  }

  return ZCL_STATUS_SUCCESS;
} /* Window_Server_Up_Cb */

/**
 * @brief  Window server Down command callback
 * @param  cluster pointer to cluster server
 * @param  zclHdrPtr ZigBee header
 * @param  dataIndPtr source addr
 * @param  arg extra arg
 * @retval stack status code
 */
enum ZclStatusCodeT Window_Server_Down_Cb
(struct ZbZclClusterT *cluster, struct ZbZclHeaderT *zclHdrPtr,
 struct ZbApsdeDataIndT *dataIndPtr, void *arg)
{
  APP_ZB_DBG("Down Command : Coming From : 0x%016llx",dataIndPtr->src.extAddr);

  // Check if a different command run to take it the new one
  if (App_Roller_Shutter_Window_Covering_Get_Cmd() != ZCL_WNCV_COMMAND_DOWN)
  {
    if ((ZbZclAttrIntegerWrite(cluster, ZCL_WNCV_SVR_ATTR_CURR_POS_LIFT_PERCENT, ZCL_WNCV_COMMAND_DOWN) != ZCL_STATUS_SUCCESS) ||
        (ZbZclAttrIntegerWrite(cluster, ZCL_WNCV_SVR_ATTR_CURR_POS_TILT_PERCENT, ZCL_WNCV_COMMAND_DOWN) != ZCL_STATUS_SUCCESS))
    {
      APP_ZB_DBG("Error ZbZclAttrIntegerWrite");
      return ZCL_STATUS_FAILURE;
    }

    App_Roller_Shutter_Window_Covering_Set_Cmd((uint8_t) ZCL_WNCV_COMMAND_DOWN);
    UTIL_SEQ_SetTask(1U << CFG_TASK_MOTOR_CONTROL, UTIL_SEQ_RFU);
  }

  return ZCL_STATUS_SUCCESS;
} /* Window_Server_Down_Cb */

/**
 * @brief  Window server Stop command callback
 * @param  cluster pointer to cluster server
 * @param  zclHdrPtr ZigBee header
 * @param  dataIndPtr source addr
 * @param  arg extra arg
 * @retval stack status code
 */
enum ZclStatusCodeT Window_Server_Stop_Cb
(struct ZbZclClusterT *cluster, struct ZbZclHeaderT *zclHdrPtr,
 struct ZbApsdeDataIndT *dataIndPtr, void *arg)
{
  APP_ZB_DBG("Stop Command : Coming From : 0x%016llx",dataIndPtr->src.extAddr);

  // Check if a different command run to take it the new one
  if (App_Roller_Shutter_Window_Covering_Get_Cmd() != ZCL_WNCV_COMMAND_STOP)
  {
    if ((ZbZclAttrIntegerWrite(cluster, ZCL_WNCV_SVR_ATTR_CURR_POS_LIFT_PERCENT, ZCL_WNCV_COMMAND_STOP) != ZCL_STATUS_SUCCESS) ||
        (ZbZclAttrIntegerWrite(cluster, ZCL_WNCV_SVR_ATTR_CURR_POS_TILT_PERCENT, ZCL_WNCV_COMMAND_STOP) != ZCL_STATUS_SUCCESS))
    { 
      APP_ZB_DBG("Error ZbZclAttrIntegerWrite");
      return ZCL_STATUS_FAILURE;
    }

    App_Roller_Shutter_Window_Covering_Set_Cmd((uint8_t) ZCL_WNCV_COMMAND_STOP);
    UTIL_SEQ_SetTask(1U << CFG_TASK_MOTOR_CONTROL, UTIL_SEQ_RFU);
  }

  return ZCL_STATUS_SUCCESS;
} /* Window_Server_Stop_Cb */

