/**
  ******************************************************************************
  * @file    app_light_level.c
  * @author  Zigbee Application Team
  * @brief   Application interface for Level functionnalities for Light EndPoint
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

/* board dependancies */
#include "stm32wb5mm_dk_lcd.h"
#include "stm32_lcd.h"

/* service dependancies */
#include "app_light_cfg.h"
#include "stm32_seq.h"

/* Private defines -----------------------------------------------------------*/

/* Application Variable-------------------------------------------------------*/
Level_Control_T app_Level =
{
  .level     = LIGHT_LEVEL_DEFAULT,
  .level_inc = true,
};

/* External variables --------------------------------------------------------*/
extern Light_Control_T app_Light_Control;

/* LevelControl callbacks Declaration --------------------------------------- */
enum ZclStatusCodeT light_level_server_step_cb(
  struct ZbZclClusterT *clusterPtr, struct ZbZclLevelClientStepReqT *req, struct ZbZclAddrInfoT *srcInfo, void *arg);
enum ZclStatusCodeT light_level_server_stop_cb(
  struct ZbZclClusterT *clusterPtr, struct ZbZclLevelClientStopReqT *req, struct ZbZclAddrInfoT *srcInfo, void *arg);

/* Application Variable------------------------------------------------------ */
/* Level Attributes persistent flag set */
static const struct ZbZclAttrT zcl_levelcontrol_server_attr_list[] =
{
  {
    ZCL_LEVEL_ATTR_CURRLEVEL, ZCL_DATATYPE_UNSIGNED_8BIT,
    ZCL_ATTR_FLAG_REPORTABLE | ZCL_ATTR_FLAG_PERSISTABLE, 0, NULL, {0, 0}, {0, 0}
  },
};

/* Clusters CFG ------------------------------------------------------------- */
/**
 * @brief  Configure and register Zigbee application endpoints, onoff callbacks
 * @param  zb Zigbee stack instance
 * @retval None
 */
Level_Control_T * App_Light_Level_Cfg(struct ZigBeeT *zb)
{
  struct ZbZclLevelServerCallbacksT LevelServerCallbacks;
  
  /* callbacks Level */
  memset(&LevelServerCallbacks, 0, sizeof(LevelServerCallbacks));
  LevelServerCallbacks.move_to_level = light_level_server_move_to_level_cb;
  LevelServerCallbacks.move          = light_level_server_move_cb;
  LevelServerCallbacks.step          = light_level_server_step_cb;
  LevelServerCallbacks.stop          = light_level_server_stop_cb;
  
  /* LevelControl server */
  app_Level.level_server = ZbZclLevelServerAlloc(zb, LIGHT_ENDPOINT,  NULL, &LevelServerCallbacks, NULL);
  assert( app_Level.level_server != NULL);
  ZbZclClusterEndpointRegister( app_Level.level_server);

  /* Allocate the attributes */
  if (ZbZclAttrAppendList( app_Level.level_server, zcl_levelcontrol_server_attr_list, ZCL_ATTR_LIST_LEN(zcl_levelcontrol_server_attr_list)))
  {
    ZbZclClusterFree( app_Level.level_server);
  }

  return &app_Level;

} /*  App_Light_Level_Cfg */

/**
 * @brief  Restore the state at startup from persistence
 * @param  None
 * @retval stack status code
 */
enum ZclStatusCodeT App_Light_Level_Restore_State(void)
{
  enum ZclStatusCodeT status = ZCL_STATUS_FAILURE;
  struct ZbZclLevelClientMoveToLevelReqT req =
  {
    .level = LIGHT_LEVEL_DEFAULT,
  };

  status = ZbZclAttrRead(app_Level.level_server, ZCL_LEVEL_ATTR_CURRLEVEL, NULL,
                       &(app_Level.level), sizeof(app_Level.level), false);
  
  /* Locally manage error if possible */
  switch(status)
  {
    case ZCL_STATUS_UNSUPP_ATTRIBUTE :
    case ZCL_STATUS_UNREPORTABLE_ATTRIBUTE :
    case ZCL_STATUS_INSUFFICIENT_SPACE :
    case ZCL_STATUS_FAILURE :
      app_Level.level = LIGHT_LEVEL_DEFAULT;
      /* Default value */
      (void) light_level_server_move_to_level_cb(app_Level.level_server, &req, NULL, NULL);
      APP_ZB_DBG("Read back cluster information : FAILED");
      status = ZCL_STATUS_FAILURE;
      break;
    
    default :
      status = ZCL_STATUS_SUCCESS;
      break;
  }

  return status;
}/* App_Light_Level_Restore_State */

/* Level callbacks Definition ----------------------------------------------- */
/**< Callback to application, invoked on receipt of Move To Level command. Set with_onoff to true in the req struct when utilizing the
     * onoff cluster on the same endpoint. The application is expected to update ZCL_LEVEL_ATTR_CURRLEVEL */
    /**
 * @brief  Level server move_to_level callback
 * @param  clusterPtr pointer to cluster,
 * @param  req structure of the command as level
 * @param  srcInfo source addr of the request command
 * @param  arg extra arg
 * @retval stack status code
 */
enum ZclStatusCodeT light_level_server_move_to_level_cb(struct ZbZclClusterT *clusterPtr, 
struct ZbZclLevelClientMoveToLevelReqT *req, struct ZbZclAddrInfoT *srcInfo, void *arg)
{
  APP_ZB_DBG("Control Level: 0x%X", req->level);
  (void)ZbZclAttrIntegerWrite(clusterPtr, ZCL_LEVEL_ATTR_CURRLEVEL, req->level);
  app_Level.level = req->level;

  //TODO take the OnOff dependancy in the command ??? if (req->with_onoff) {}  
  
  UTIL_SEQ_SetTask(1U << CFG_TASK_LIGHT_UPDATE, CFG_SCH_PRIO_1);
   
  return ZCL_STATUS_SUCCESS;
} /* light_level_server_move_to_level_cb */

/* Level server move command callback */
enum ZclStatusCodeT light_level_server_move_cb(struct ZbZclClusterT *clusterPtr, struct ZbZclLevelClientMoveReqT *req, struct ZbZclAddrInfoT *srcInfo, void *arg)
{
  /* Select mode to apply */
  switch (req->mode)
  {
    case LIGHT_LEVEL_MODE_DOWN :
      if ( app_Level.level > LIGHT_LEVEL_MIN_VALUE)
      {
        APP_ZB_DBG("Level Ctrl Down : 0x%x --> 0x%x", app_Level.level, app_Level.level - LIGHT_LEVEL_STEP);
        app_Level.level -= LIGHT_LEVEL_STEP;
      }
      else
      {
        APP_ZB_DBG("Min Level already reached");
      }
      break;
      
    case LIGHT_LEVEL_MODE_UP :
      if ( app_Level.level == LIGHT_LEVEL_MAX_VALUE)
      {
        APP_ZB_DBG("Max Level already reached");
      }
      /* Check if overflow happens */
      else if ((uint16_t) ( app_Level.level + LIGHT_LEVEL_STEP) > LIGHT_LEVEL_MAX_VALUE)
      {
        APP_ZB_DBG("Level Ctrl Up : 0x%x --> 0x%x", app_Level.level, LIGHT_LEVEL_MAX_VALUE);
        app_Level.level = LIGHT_LEVEL_MAX_VALUE ;
      }
      else
      {
        APP_ZB_DBG("Level Ctrl Up : 0x%x --> 0x%x", app_Level.level, app_Level.level + LIGHT_LEVEL_STEP);
         app_Level.level += LIGHT_LEVEL_STEP;
      }
      break;
      
    default :
      APP_ZB_DBG("Eror: Unknown level command mode : %d",req->mode);
      break;
  }
  
  (void)ZbZclAttrIntegerWrite(clusterPtr, ZCL_LEVEL_ATTR_CURRLEVEL,  app_Level.level);
  
  UTIL_SEQ_SetTask(1U << CFG_TASK_LIGHT_UPDATE, CFG_SCH_PRIO_1);

  return ZCL_STATUS_SUCCESS;
} /* levelControl_server_move_cb */

/* Level server step command callback */
enum ZclStatusCodeT light_level_server_step_cb(struct ZbZclClusterT *clusterPtr, struct ZbZclLevelClientStepReqT *req, struct ZbZclAddrInfoT *srcInfo, void *arg)
{
  return ZCL_STATUS_SUCCESS;
}

/* Level server stop command callback */
enum ZclStatusCodeT light_level_server_stop_cb(struct ZbZclClusterT *clusterPtr, struct ZbZclLevelClientStopReqT *req, struct ZbZclAddrInfoT *srcInfo, void *arg)
{
  return ZCL_STATUS_SUCCESS;
}


