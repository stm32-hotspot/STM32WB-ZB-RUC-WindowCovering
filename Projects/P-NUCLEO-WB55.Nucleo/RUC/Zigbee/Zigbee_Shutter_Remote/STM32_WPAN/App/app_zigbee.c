/**
  ******************************************************************************
  * @file    app_zigbee.c
  * @author  Zigbee Application Team
  * @brief   Zigbee Part only
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
#include "app_common.h"
#include "tl_zigbee_hci.h"
#include "shci.h"
#include "stm32wbxx_core_interface_def.h"
#include "stm32_seq.h"

/* Debug Part */
#include <assert.h>
#include "stm_logging.h"
#include "dbg_trace.h"

/* service dependencies */
#include "app_core.h"
#include "app_nvm.h"
#include "app_zigbee.h"

/* Private defines -----------------------------------------------------------*/
#define APP_ZIGBEE_STARTUP_FAIL_DELAY  500U
// #define CHANNEL                        25
// #define CHANNELMASK_USED               (1<< CHANNEL)
#define CHANNELMASK_USED               WPAN_CHANNELMASK_2400MHZ; /* Full Channel in use */

/* Private function prototypes -----------------------------------------------*/
static enum ZbStatusCodeT ZbStartupWait(struct ZigBeeT *zb, struct ZbStartupT *config);
static void App_Zigbee_NwkJoin         (void);
static uint8_t App_Zigbee_Get_Channel  (uint32_t mask, uint16_t *first_channel);
static void App_Zigbee_Set_TxPwr       (int8_t updated_val_tx_power);
static void App_Zigbee_Unbind_cb       (struct ZbZdoBindRspT *rsp, void *cb_arg);
static void App_Zigbee_TraceError      (const char *pMess, uint32_t ErrCode);

/* M4-M0 communication */
static void Wait_Getting_Ack_From_M0    (void);
static void Receive_Ack_From_M0         (void);
static void Receive_Notification_From_M0(void);

/* Private variables ---------------------------------------------------------*/
static TL_CmdPacket_t * p_ZIGBEE_otcmdbuffer;
static TL_EvtPacket_t * p_ZIGBEE_notif_M0_to_M4;
static TL_EvtPacket_t * p_ZIGBEE_request_M0_to_M4;
static __IO uint32_t    CptReceiveNotifyFromM0 = 0;
static __IO uint32_t    CptReceiveRequestFromM0 = 0;

/* Buffer memories */
PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_ZIGBEE_Config_t ZigbeeConfigBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t     ZigbeeOtCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ZigbeeNotifRspEvtBuffer [sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ZigbeeNotifRequestBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];


/* Shared variables -------------------------------------------------------- */
/* zigbee info, it could be used globally for the application */
App_Zb_Info_T app_zb_info =
{
  .tx_power = 0,
};


/* Functions Definition ------------------------------------------------------*/
/**
 * @brief  Zigbee part initialization
 * Setting different services needed to use/call stack and network Zigbee
 * 
 * @param  None
 * @retval None
 */
void App_Zigbee_Init(void)
{
  SHCI_CmdStatus_t ZigbeeInitStatus;

  APP_ZB_DBG("App_Zigbee_Init");

  /* Check the compatibility with the Coprocessor Wireless Firmware loaded */
  APP_ZB_DBG("**********************************************************");
  App_Zigbee_Check_Firmware_Info();
  APP_ZB_DBG("**********************************************************");

  /* Register cmdbuffer */
  App_Zigbee_RegisterCmdBuffer(&ZigbeeOtCmdBuffer);

  /* Init config buffer and call TL_ZIGBEE_Init */
  App_Zigbee_TL_INIT();

  /* Flash Init */
  App_NVM_Init();

  /* Create the different tasks */
  UTIL_SEQ_RegTask(1U << (uint32_t)CFG_TASK_NOTIFY_FROM_M0_TO_M4,  UTIL_SEQ_RFU, App_Zigbee_ProcessNotifyM0ToM4);
  UTIL_SEQ_RegTask(1U << (uint32_t)CFG_TASK_REQUEST_FROM_M0_TO_M4, UTIL_SEQ_RFU, App_Zigbee_ProcessRequestM0ToM4);

  /* Task associated with network creation process */
  UTIL_SEQ_RegTask(1U << (uint32_t)CFG_TASK_ZIGBEE_NETWORK_JOIN, UTIL_SEQ_RFU, App_Zigbee_NwkJoin);

  /* Start the Zigbee on the CPU2 side */
  ZigbeeInitStatus = SHCI_C2_ZIGBEE_Init();
  /* Prevent unused argument(s) compilation warning */
  UNUSED(ZigbeeInitStatus);
} /* App_Zigbee_Init */

/**
 * @brief Check and display the M0 Firmware type loaded,
 *        the application name and the Link key value
 * 
 * @param  None
 * @retval None
 */
void App_Zigbee_Check_Firmware_Info(void)
{
  WirelessFwInfo_t wireless_info_instance;
  WirelessFwInfo_t *p_wireless_info = &wireless_info_instance;

  if (SHCI_GetWirelessFwInfo(p_wireless_info) != SHCI_Success)
  {
    App_Zigbee_Error((uint32_t)ERR_ZIGBEE_CHECK_WIRELESS, (uint32_t)ERR_INTERFACE_FATAL);
  }
  else
  {
    APP_ZB_DBG("WIRELESS COPROCESSOR FW:");
    /* Print version */
    APP_ZB_DBG("VERSION ID = %d.%d.%d", p_wireless_info->VersionMajor, p_wireless_info->VersionMinor, p_wireless_info->VersionSub);

    switch (p_wireless_info->StackType)
    {
      case INFO_STACK_TYPE_ZIGBEE_FFD:
        APP_ZB_DBG("FW Type : FFD Zigbee stack");
        break;
      case INFO_STACK_TYPE_ZIGBEE_RFD:
        APP_ZB_DBG("FW Type : RFD Zigbee stack");
        break;
      default:
        /* No Zigbee device supported ! */
        App_Zigbee_Error((uint32_t)ERR_ZIGBEE_CHECK_WIRELESS, (uint32_t)ERR_INTERFACE_FATAL);
        break;
    }

    // print the application name
    char* __PathProject__ =(strstr(__FILE__, "Zigbee_") ? strstr(__FILE__, "Zigbee_") : __FILE__);
    char *del;
    if ( (strchr(__FILE__, '/')) == NULL)
    {del = strchr(__PathProject__, '\\');}
    else
    {del = strchr(__PathProject__, '/');}
    
    int index = (int) (del - __PathProject__);
    APP_ZB_DBG("Application flashed: %*.*s",index,index,__PathProject__);
    
    //print Link Key
    APP_ZB_DBG("Link Key: %.16s", sec_key_ha);
    //print Link Key value hex   
    char Z09_LL_string[ZB_SEC_KEYSIZE*3+1];
    Z09_LL_string[0]=0;
    for(int str_index=0; str_index < ZB_SEC_KEYSIZE; str_index++)
    {
      sprintf(&Z09_LL_string[str_index*3],"%02x ",sec_key_ha[str_index]);
    }
    APP_ZB_DBG("Link Key value: %s",Z09_LL_string);
  }
} /* App_Zigbee_Check_Firmware_Info */

/**
 * @brief  Initialize Zigbee stack instance for the device
 * @param  None
 * @retval None
 */
void App_Zigbee_StackLayersInit(void)
{
  APP_ZB_DBG("App_Zigbee_StackLayersInit");

  app_zb_info.zb = ZbInit(0U, NULL, NULL); 
  assert(app_zb_info.zb != NULL);

  /* Create the endpoint(s) and cluster(s) associated from the Core */
  App_Core_ConfigEndpoints();

  /* Configure the joining parameters */
  app_zb_info.join_status = ZCL_STATUS_FAILURE; /* init to error status */
  app_zb_info.join_delay  = HAL_GetTick();      /* now */

  /* First we disable the persistent notification */
  ZbPersistNotifyRegister(app_zb_info.zb, NULL, NULL);

  /* Call a startup from persistence */
  app_zb_info.join_status = App_Startup_Persist(app_zb_info.zb);
  if (app_zb_info.join_status == ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG("SUCCESS restart from persistence");
    BSP_LED_On(LED_BLUE);
    BSP_LED_On(LED_GREEN);
    HAL_Delay(300);
    BSP_LED_Off(LED_BLUE);
    BSP_LED_Off(LED_GREEN);
    HAL_Delay(300);
    BSP_LED_On(LED_BLUE);
    BSP_LED_On(LED_GREEN);
    HAL_Delay(300);
    BSP_LED_Off(LED_BLUE);
    BSP_LED_Off(LED_GREEN);
  }
  else
  {
    APP_ZB_DBG("FAILED to restart from persistence with status: 0x%02x",app_zb_info.join_status);
  }
} /* App_Zigbee_StackLayersInit */

/**
 * @brief  Handle Zigbee network joining
 * @param  None
 * @retval None
 */
static void App_Zigbee_NwkJoin(void)
{
  if ((app_zb_info.join_status != ZB_STATUS_SUCCESS) && (HAL_GetTick() >= app_zb_info.join_delay))
  {
    struct ZbStartupT config;
   
    /* Configure Zigbee Logging (only need to do this once, but this is a good place to put it) */
    ZbSetLogging(app_zb_info.zb, ZB_LOG_MASK_LEVEL_5, NULL);

    /* Attempt to join a zigbee network */
    ZbStartupConfigGetProDefaults(&config);

    APP_ZB_DBG("Network config : APP_STARTUP_ED");
    config.startupControl = ZbStartTypeJoin;

    /* Using the default HA preconfigured Link Key */
    memcpy(config.security.preconfiguredLinkKey, sec_key_ha, ZB_SEC_KEYSIZE);   
  
    /* Multi-channel prog */
    config.channelList.count = 1;
    config.channelList.list[0].page = 0;
    config.channelList.list[0].channelMask = CHANNELMASK_USED; /* Channel in use*/

    /* Using ZbStartupWait (blocking) here instead of ZbStartup, in order to demonstrate how to do
     * a blocking call on the M4. */
    app_zb_info.join_status = ZbStartupWait(app_zb_info.zb, &config);
    APP_ZB_DBG("ZbStartup Callback (status = 0x%02x)", app_zb_info.join_status);

    if (app_zb_info.join_status == ZB_STATUS_SUCCESS)
    {
      app_zb_info.join_delay = 0U;
      /* Register Persistent data change notification */
      ZbPersistNotifyRegister(app_zb_info.zb, App_Persist_Notify_cb, NULL);
      /* Call the callback once here to save persistence data */
      App_Persist_Notify_cb(app_zb_info.zb, NULL);
      /* flash x2 Green LED to inform the joining connection*/
      BSP_LED_On(LED_GREEN);
      HAL_Delay(300);
      BSP_LED_Off(LED_GREEN);
      HAL_Delay(300);
      BSP_LED_On(LED_GREEN);
      HAL_Delay(300);
      BSP_LED_Off(LED_GREEN);
    }
    else
    {
      APP_ZB_DBG("Startup failed, attempting again to join the network after a short delay (%d ms)", APP_ZIGBEE_STARTUP_FAIL_DELAY);
      app_zb_info.join_delay = HAL_GetTick() + APP_ZIGBEE_STARTUP_FAIL_DELAY;
    }
  }

  /* Indicates that the Task to form the Network is done */
  UTIL_SEQ_SetEvt(EVENT_ZIGBEE_NETWORK_JOIN); 
} /* App_Zigbee_NwkJoin */

/**
 * @brief  Get the Zigbee network channel used
 * @param  mask channel mask
 * @param  first_channel first channel found
 * @retval numbers of channel found
 */
static uint8_t App_Zigbee_Get_Channel(uint32_t mask, uint16_t *first_channel)
{
  uint8_t i, num_channels = 0;

  *first_channel = 0xff;
  for (i = 0; i < WPAN_PAGE_CHANNELS_MAX; i++)
  {
    if (((1 << i) & mask))
    {
      if (num_channels == 0U)
      {
        *first_channel = i;
      }
      num_channels++;
    }
  }
  return num_channels;
} /* App_Zigbee_Get_Channel */

/**
 * @brief  Update Display Channel used
 * 
 */
void App_Zigbee_Channel_Disp(void)
{
  struct ZbChannelListT channelList;
  unsigned int i;

  /* Get the channel number used for the network and display it */
  (void)ZbNwkGet(app_zb_info.zb, ZB_NWK_NIB_ID_ActiveChannelList, &channelList, sizeof(struct ZbChannelListT));
  for (i = 0; i < channelList.count; i++)
  {
    uint8_t num_channels;
    uint16_t active_channel;

    num_channels = App_Zigbee_Get_Channel(channelList.list[i].channelMask, &active_channel);

    /* Display channel information to user */
    if (num_channels == 1)
    {
      /* active channel is available */
      APP_ZB_DBG("Join Channel : %2d", active_channel);
    }
  }
} /* App_Zigbee_Channel_Disp */

/**
 * @brief Display the short and Extended address of the device
 * 
 */
void App_Zigbee_All_Address_Disp(void)
{
  uint16_t ShortAddr = 0U;
  uint64_t ExtAddr   = 0U;

  if (app_zb_info.join_status == ZB_STATUS_SUCCESS)
  {  
    ShortAddr = ZbShortAddress(app_zb_info.zb);
    if (ShortAddr != 0)
      APP_ZB_DBG("Short Address    : 0x%0x", ShortAddr);
    ExtAddr   = ZbExtendedAddress(app_zb_info.zb);
    if (ExtAddr != 0)
      APP_ZB_DBG("Extended Address : 0x%016llx", ExtAddr);
  }
}

/**
 * @brief Set the Tx Power to the value
 * @param  None
 * @retval None
 */
static void App_Zigbee_Set_TxPwr(int8_t updated_val_tx_power)
{
  int8_t new_tx_power = 0;
  
  /* Check if the Zigbee Stack is initialised */
  if (app_zb_info.zb == NULL)
  {
    return;
  }

  /* Check the Tx Power saturation */
  if (updated_val_tx_power <= TX_POWER_MAX)
  {
    new_tx_power = updated_val_tx_power;
  }
  else
  {
    new_tx_power = TX_POWER_MAX;
  }

  // Set the new Tx Power
  if (ZbNwkIfSetTxPower(app_zb_info.zb, "wpan0", new_tx_power) == true)
  {
    app_zb_info.tx_power = new_tx_power; /* Update internal value */
    App_Zigbee_TxPwr_Disp();
  }
} /* App_Zigbee_Set_TxPwr */

/**
 * @brief Increment the Tx Power by TX_POWER_STEP, Up to TX_POWER_MAX
 * @param  None
 * @retval None
 */
void App_Zigbee_TxPwr_Up(void)
{
  App_Zigbee_Set_TxPwr(app_zb_info.tx_power + TX_POWER_STEP);
} /* App_Zigbee_TxPwr_Up */

/**
 * @brief Decrement the Tx Power by TX_POWER_STEP, Down to TX_POWER_MIN
 * @param  None
 * @retval None
 */
void App_Zigbee_TxPwr_Down(void)
{
  App_Zigbee_Set_TxPwr(app_zb_info.tx_power - TX_POWER_STEP);
} /* App_Zigbee_TxPwr_Down */

/**
 * @brief Display the new Value of Tx Power on LCD
 * @param  None
 * @retval None
 */
void App_Zigbee_TxPwr_Disp(void)
{
  APP_ZB_DBG("Tx Power : %2d dbm", app_zb_info.tx_power);
} /* App_Zigbee_TxPwr_Disp */

/**
 * @brief Send an Unbind Req foreach device of the binding table 
 * to Use with the Factory reset to clean the network before leave it
 * 
 * @param  None
 * @retval None
 */
void App_Zigbee_Unbind_All(void)
{
  struct ZbApsmeBindT  entry;
  struct ZbZdoBindReqT req;
  enum   ZbStatusCodeT status;

  APP_ZB_DBG("Unbind Devices below");
  APP_ZB_DBG(" Item | Long Address     |  ClusterId |  EP  |");
  APP_ZB_DBG(" -----|------------------|------------|------|");
  /* go through each elements to unbind them */
  for (uint8_t idx = 0; ; idx++)
  {
    /* check the end of the table */
    if (ZbApsGetIndex(app_zb_info.zb, ZB_APS_IB_ID_BINDING_TABLE, &entry, sizeof(entry), idx) != ZB_APS_STATUS_SUCCESS)
    {
      break;
    }
    /* remove itself */
    if (entry.srcExtAddr == 0ULL)
    {
      continue;
    }

    /* Invert src/dst from binding element to create unbind request */
    req.srcEndpt     = entry.dst.endpoint;
    req.srcExtAddr   = entry.dst.extAddr;
    req.dst.mode     = ZB_APSDE_ADDRMODE_EXT;
    req.dst.extAddr  = entry.srcExtAddr;
    req.dst.endpoint = entry.srcEndpt;
    req.clusterId    = entry.clusterId;
    /* be careful the API uses the target network address to send Unbind request (as Bind request) */
    req.target       = ZbNwkAddrLookupNwk(app_zb_info.zb, entry.dst.extAddr);
    /* check if resolution address found */
    if (req.target == ZB_NWK_ADDR_UNDEFINED)
    {
      APP_ZB_DBG("[Unbind Req] Nwk Add unknow");
      return;
    }
    /* display element and unbind it */
    APP_ZB_DBG("  %2d  | %016llx |    0x%03X   | 0x%02X |", idx, entry.dst.extAddr, entry.clusterId, entry.srcEndpt);
    status = ZbZdoUnbindReq(app_zb_info.zb, &req, &App_Zigbee_Unbind_cb, &req);
    if (status != ZB_STATUS_SUCCESS)
    {
      APP_ZB_DBG("UnbindReq status 0x%02X", status);
    }
  }
} /* App_Zigbee_Unbind_All */

/**
 * @brief Analyse the Device response for the Unbind Request
 *  
 * @param rsp from the device at the UnbindReq command
 * @param cb_arg unbind request to analyse
 */
static void App_Zigbee_Unbind_cb(struct ZbZdoBindRspT *rsp, void *cb_arg)
{
  struct ZbZdoBindReqT * req = cb_arg;

  if (rsp->status != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Unbind Request Failed 0x%02X, investigate with following Request", rsp->status);
    APP_ZB_DBG(" Long Address     |  ClusterId |  EP  |");
    APP_ZB_DBG("------------------|------------|------|");
    APP_ZB_DBG(" %016llx |    0x%03X  | 0x%02X |", req->dst.extAddr, req->clusterId, req->srcEndpt);
    APP_ZB_DBG("");
  }
} /* App_Zigbee_Unbind_cb */

/**
 * @brief  For debug purpose, display the local binding table information
 * @param  None 
 * @retval None
 */
void App_Zigbee_Bind_Disp(void)
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
    if (ZbApsGetIndex(app_zb_info.zb, ZB_APS_IB_ID_BINDING_TABLE, &entry, sizeof(entry), i) != ZB_APS_STATUS_SUCCESS)
    {
      break;
    }
    if (entry.srcExtAddr == 0ULL)
    {
      continue;
    }
    APP_ZB_DBG("  %2d  | %016llx |   0x%04x  |    0x%04x    |    0x%04x", i, entry.dst.extAddr, entry.clusterId, entry.srcEndpt, entry.dst.endpoint);
  }
  APP_ZB_DBG("------------------------------------------------------------------\n\r");  
} /* App_Zigbee_Bind_Disp */


/*************************************************************
 * ZbStartupWait Blocking Call
 *************************************************************/
struct ZbStartupWaitInfo
{
  bool active;
  enum ZbStatusCodeT status;
};

static void ZbStartupWaitCb(enum ZbStatusCodeT status, void *arg)
{
  struct ZbStartupWaitInfo *info = arg;

  info->status = status;
  info->active = false;
  UTIL_SEQ_SetEvt(EVENT_ZIGBEE_STARTUP_ENDED);
} /* ZbStartupWaitCb */

/**
 * @brief  startup wait function
 * @param  zb Zigbee stack pointer
 * @param  config startup config pointer
 * @retval zigbee status stack code
 */
static enum ZbStatusCodeT ZbStartupWait(struct ZigBeeT *zb, struct ZbStartupT *config)
{
  struct ZbStartupWaitInfo *info;
  enum ZbStatusCodeT status;

  info = malloc(sizeof(struct ZbStartupWaitInfo));
  if (info == NULL)
  {
    return ZB_STATUS_ALLOC_FAIL;
  }
  memset(info, 0, sizeof(struct ZbStartupWaitInfo));

  info->active = true;
  status = ZbStartup(zb, config, ZbStartupWaitCb, info);
  if (status != ZB_STATUS_SUCCESS)
  {
    info->active = false;
    return status;
  }
  UTIL_SEQ_WaitEvt(EVENT_ZIGBEE_STARTUP_ENDED);
  status = info->status;
  free(info);
  return status;
} /* ZbStartupWait */


/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief  Trace the error or the warning reported.
 * @param  ErrId
 * @param  ErrCode
 * @retval None
 */
void App_Zigbee_Error(uint32_t ErrId, uint32_t ErrCode)
{
  switch (ErrId)
  {
    default:
      App_Zigbee_TraceError("ERROR Unknown ", 0);
      break;
  }
} /* App_Zigbee_Error */

/**
 * @brief  Warn the user that an error has occurred.In this case,
 *         the LEDs on the Board will start blinking.
 *
 * @param  pMess  : Message associated to the error.
 * @param  ErrCode: Error code associated to the module (Zigbee or other module if any)
 * @retval None
 */
static void App_Zigbee_TraceError(const char *pMess, uint32_t ErrCode)
{
  APP_ZB_DBG("**** Fatal error = %s (Err = %d)", pMess, ErrCode);
  while (1U == 1U)
  {
    BSP_LED_Toggle(LED1);
    HAL_Delay(500U);
    BSP_LED_Toggle(LED2);
    HAL_Delay(500U);
    BSP_LED_Toggle(LED3);
    HAL_Delay(500U);
  }
} /* App_Zigbee_TraceError */


/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

void App_Zigbee_RegisterCmdBuffer(TL_CmdPacket_t *p_buffer)
{
  p_ZIGBEE_otcmdbuffer = p_buffer;
} /* App_Zigbee_RegisterCmdBuffer */

Zigbee_Cmd_Request_t * ZIGBEE_Get_OTCmdPayloadBuffer(void)
{
  return (Zigbee_Cmd_Request_t *)p_ZIGBEE_otcmdbuffer->cmdserial.cmd.payload;
} /* ZIGBEE_Get_OTCmdPayloadBuffer */

Zigbee_Cmd_Request_t * ZIGBEE_Get_OTCmdRspPayloadBuffer(void)
{
  return (Zigbee_Cmd_Request_t *)((TL_EvtPacket_t *)p_ZIGBEE_otcmdbuffer)->evtserial.evt.payload;
} /* ZIGBEE_Get_OTCmdRspPayloadBuffer */

Zigbee_Cmd_Request_t * ZIGBEE_Get_NotificationPayloadBuffer(void)
{
  return (Zigbee_Cmd_Request_t *)(p_ZIGBEE_notif_M0_to_M4)->evtserial.evt.payload;
} /* ZIGBEE_Get_NotificationPayloadBuffer */

Zigbee_Cmd_Request_t * ZIGBEE_Get_M0RequestPayloadBuffer(void)
{
  return (Zigbee_Cmd_Request_t *)(p_ZIGBEE_request_M0_to_M4)->evtserial.evt.payload;
}

/**
 * @brief  This function is used to transfer the commands from the M4 to the M0.
 * @param   None
 * @return  None
 */
void ZIGBEE_CmdTransfer(void)
{
  Zigbee_Cmd_Request_t *cmd_req = (Zigbee_Cmd_Request_t *)p_ZIGBEE_otcmdbuffer->cmdserial.cmd.payload;

  /* Zigbee OT command cmdcode range 0x280 .. 0x3DF = 352 */
  p_ZIGBEE_otcmdbuffer->cmdserial.cmd.cmdcode = 0x280U;
  /* Size = otCmdBuffer->Size (Number of OT cmd arguments : 1 arg = 32bits so multiply by 4 to get size in bytes)
   * + ID (4 bytes) + Size (4 bytes) */
  p_ZIGBEE_otcmdbuffer->cmdserial.cmd.plen = 8U + (cmd_req->Size * 4U);

  TL_ZIGBEE_SendM4RequestToM0();

  /* Wait completion of cmd */
  Wait_Getting_Ack_From_M0();
} /* ZIGBEE_CmdTransfer */

/**
 * @brief  This function is called when the M0+ acknowledge  the fact that it has received a Cmd
 * @param   Otbuffer pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_ZIGBEE_CmdEvtReceived(TL_EvtPacket_t *Otbuffer)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Otbuffer);

  Receive_Ack_From_M0();
} /* TL_ZIGBEE_CmdEvtReceived */

/**
 * @brief  This function is called when notification from M0+ is received.
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_ZIGBEE_NotReceived(TL_EvtPacket_t *Notbuffer)
{
  p_ZIGBEE_notif_M0_to_M4 = Notbuffer;

  Receive_Notification_From_M0();
} /* TL_ZIGBEE_NotReceived */

/**
 * @brief  This function is called before sending any ot command to the M0
 *         core. The purpose of this function is to be able to check if
 *         there are no notifications coming from the M0 core which are
 *         pending before sending a new ot command.
 * @param  None
 * @retval None
 */
void Pre_ZigbeeCmdProcessing(void)
{
  UTIL_SEQ_WaitEvt(EVENT_SYNCHRO_BYPASS_IDLE);
} /* Pre_ZigbeeCmdProcessing */

/**
 * @brief  This function waits for getting an acknowledgment from the M0.
 * @param  None
 * @retval None
 */
static void Wait_Getting_Ack_From_M0(void)
{
  UTIL_SEQ_WaitEvt(EVENT_ACK_FROM_M0_EVT);
} /* Wait_Getting_Ack_From_M0 */

/**
 * @brief  Receive an acknowledgment from the M0+ core.
 *         Each command send by the M4 to the M0 are acknowledged.
 *         This function is called under interrupt.
 * @param  None
 * @retval None
 */
static void Receive_Ack_From_M0(void)
{
  UTIL_SEQ_SetEvt(EVENT_ACK_FROM_M0_EVT);
} /* Receive_Ack_From_M0 */

/**
 * @brief  Receive a notification from the M0+ through the IPCC.
 *         This function is called under interrupt.
 * @param  None
 * @retval None
 */
static void Receive_Notification_From_M0(void)
{
  CptReceiveNotifyFromM0++;
  UTIL_SEQ_SetTask(1U << (uint32_t)CFG_TASK_NOTIFY_FROM_M0_TO_M4, CFG_SCH_PRIO_0);
}

/**
 * @brief  This function is called when a request from M0+ is received.
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_ZIGBEE_M0RequestReceived(TL_EvtPacket_t *Reqbuffer)
{
  p_ZIGBEE_request_M0_to_M4 = Reqbuffer;

  CptReceiveRequestFromM0++;
  UTIL_SEQ_SetTask(1U << (uint32_t)CFG_TASK_REQUEST_FROM_M0_TO_M4, CFG_SCH_PRIO_0);
}

/**
 * @brief Perform initialization of TL for Zigbee.
 * @param  None
 * @retval None
 */
void App_Zigbee_TL_INIT(void)
{
  ZigbeeConfigBuffer.p_ZigbeeOtCmdRspBuffer     = (uint8_t *)&ZigbeeOtCmdBuffer;
  ZigbeeConfigBuffer.p_ZigbeeNotAckBuffer       = (uint8_t *) ZigbeeNotifRspEvtBuffer;
  ZigbeeConfigBuffer.p_ZigbeeNotifRequestBuffer = (uint8_t *) ZigbeeNotifRequestBuffer;
  TL_ZIGBEE_Init(&ZigbeeConfigBuffer);
} /* App_Zigbee_TL_INIT */

/**
 * @brief Process the messages coming from the M0.
 * @param  None
 * @retval None
 */
void App_Zigbee_ProcessNotifyM0ToM4(void)
{
  if (CptReceiveNotifyFromM0 != 0)
  {
    /* If CptReceiveNotifyFromM0 is > 1. it means that we did not serve all the events from the radio */
    if (CptReceiveNotifyFromM0 > 1U)
    {
      App_Zigbee_Error(ERR_REC_MULTI_MSG_FROM_M0, 0);
    }
    else
    {
      Zigbee_CallBackProcessing();
    }
    /* Reset counter */
    CptReceiveNotifyFromM0 = 0;
  }
} /* App_Zigbee_ProcessNotifyM0ToM4 */

/**
 * @brief Process the requests coming from the M0.
 * @param  None
 * @return None
 */
void App_Zigbee_ProcessRequestM0ToM4(void)
{
  if (CptReceiveRequestFromM0 != 0)
  {
    Zigbee_M0RequestProcessing();
    CptReceiveRequestFromM0 = 0;
  }
} /* App_Zigbee_ProcessRequestM0ToM4 */

