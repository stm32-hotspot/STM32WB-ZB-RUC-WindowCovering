/**
  ******************************************************************************
  * @file    app_light_onoff.c
  * @author  Zigbee Application Team
  * @brief   Application interface for OnOff functionnality
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
OnOff_Control_T app_OnOff =
{
  .On      =  0x0U,
};

/* OnOff callbacks Declaration --------------------------------------------- */
static enum ZclStatusCodeT light_onoff_server_off_cb(
  struct ZbZclClusterT *clusterPtr, struct ZbZclAddrInfoT *srcInfo, void *arg);
static enum ZclStatusCodeT light_onoff_server_on_cb(
  struct ZbZclClusterT *clusterPtr, struct ZbZclAddrInfoT *srcInfo, void *arg);

/* Application Variable----------------------------------------------------- */
/* On Off  Attributes persistent flag set */
const struct ZbZclAttrT zcl_onoff_server_attr_list[] = 
{
  {
    ZCL_ONOFF_ATTR_ONOFF, ZCL_DATATYPE_BOOLEAN,
    ZCL_ATTR_FLAG_REPORTABLE|ZCL_ATTR_FLAG_PERSISTABLE, 0, NULL, {0, 0}, {0, 0}
  },
};


/* Clusters CFG ------------------------------------------------------------ */
/**
 * @brief  Configure and register Zigbee application endpoints, onoff callbacks
 * @param  zb Zigbee stack instance
 * @retval None
 */
OnOff_Control_T * App_Light_OnOff_Cfg(struct ZigBeeT *zb)
{
  struct ZbZclOnOffServerCallbacksT onOffServerCb;
  
  /* callbacks onoff */
  memset(&onOffServerCb, 0, sizeof(onOffServerCb));
  onOffServerCb.off    = light_onoff_server_off_cb;
  onOffServerCb.on     = light_onoff_server_on_cb;
  onOffServerCb.toggle = light_onoff_server_toggle_cb;
  
  /* OnOff Server */
  app_OnOff.onoff_server = ZbZclOnOffServerAlloc(zb, LIGHT_ENDPOINT,&onOffServerCb,NULL);
  assert(app_OnOff.onoff_server != NULL);
  if (ZbZclClusterEndpointRegister(app_OnOff.onoff_server) == false)
  {
    APP_ZB_DBG("Error while Registering on off server");
  }
  
  /* OnOff attributes */
  if (ZbZclAttrAppendList(app_OnOff.onoff_server, zcl_onoff_server_attr_list, ZCL_ATTR_LIST_LEN(zcl_onoff_server_attr_list))) 
  {
    ZbZclClusterFree(app_OnOff.onoff_server);
  }

  return &app_OnOff;

} /* App_Light_OnOff_Cfg */

/**
 * @brief  Restore the state at startup from persistence
 * @param  None
 * @retval stack status code
 */
enum ZclStatusCodeT App_Light_OnOff_Restore_State(void)
{
  enum ZclStatusCodeT status = ZCL_STATUS_FAILURE;
  status = ZbZclAttrRead(app_OnOff.onoff_server, ZCL_ONOFF_ATTR_ONOFF, NULL, 
                        &(app_OnOff.On), sizeof(app_OnOff.On), true);

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
} /* App_Light_OnOff_Restore_State */


/* OnOff callbacks Definition ---------------------------------------------- */
/**
 * @brief  OnOff server Off command callback
 * @param  clusterPtr pointer to cluster
 * @param  srcInfo source addr
 * @param  arg extra arg
 * @retval stack status code, laurent@capella.fr
 */
static enum ZclStatusCodeT light_onoff_server_off_cb(
  struct ZbZclClusterT *clusterPtr, struct ZbZclAddrInfoT *srcInfo, void *arg)
{ 
  // if (app_OnOff.On == 0)
  // {
  //   APP_ZB_DBG("Error : Report not received by client 0x%016llx",srcInfo->addr.extAddr);
  //   return ZCL_STATUS_SUCCESS;    
  // }
  
  APP_ZB_DBG("LED OFF : Coming From : 0x%016llx",srcInfo->addr.extAddr);
  app_OnOff.On=0;
  (void) ZbZclAttrIntegerWrite(clusterPtr, ZCL_ONOFF_ATTR_ONOFF, 0);
  UTIL_SEQ_SetTask(1U << CFG_TASK_LIGHT_UPDATE, CFG_SCH_PRIO_1);

  return ZCL_STATUS_SUCCESS;
} /* light_onoff_server_off_cb */

/**
 * @brief  OnOff server On command callback
 * @param  clusterPtr pointer to cluster
 * @param  srcInfo source addr
 * @param  arg extra arg
 * @retval stack status code
 */
static enum ZclStatusCodeT light_onoff_server_on_cb(
  struct ZbZclClusterT *clusterPtr, struct ZbZclAddrInfoT *srcInfo, void *arg)
{ 
  // if (app_OnOff.On == 1)
  // {
  //   APP_ZB_DBG("Error : Report not received by client 0x%016llx",srcInfo->addr.extAddr);
  //   return ZCL_STATUS_FAILURE;
  // }
  
  APP_ZB_DBG("LED ON  : Coming From : 0x%016llx",srcInfo->addr.extAddr);
  app_OnOff.On = 1;
  (void) ZbZclAttrIntegerWrite(clusterPtr, ZCL_ONOFF_ATTR_ONOFF, 1);
  UTIL_SEQ_SetTask(1U << CFG_TASK_LIGHT_UPDATE, CFG_SCH_PRIO_1);

  return ZCL_STATUS_SUCCESS;
} /* light_onoff_server_on_cb */

/**
 * @brief  OnOff server toggle command callback
 * @param  clusterPtr pointer to cluster
 * @param  srcInfo source addr
 * @param  arg extra arguments for callbacks
 * @retval stack status code
 */
enum ZclStatusCodeT light_onoff_server_toggle_cb(
  struct ZbZclClusterT *clusterPtr, struct ZbZclAddrInfoT *srcInfo, void *arg)
{
  uint8_t attrVal;

  if (ZbZclAttrRead(clusterPtr, ZCL_ONOFF_ATTR_ONOFF, NULL,
      &attrVal, sizeof(attrVal), false) != ZCL_STATUS_SUCCESS)
  {
    return ZCL_STATUS_FAILURE;
  }
  APP_ZB_DBG("LED TOGGLE");
  if (attrVal != 0)
  {
    return light_onoff_server_off_cb(clusterPtr, srcInfo, arg);
  }
  else 
  {
    return light_onoff_server_on_cb(clusterPtr, srcInfo, arg);
  }
} /* light_onoff_server_toggle_cb */

