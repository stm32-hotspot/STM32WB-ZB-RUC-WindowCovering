/**
  ******************************************************************************
  * @file    app_nvm.c
  * @author  Zigbee Application Team
  * @brief   User Application file 
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
#include "app_nvm.h"

/* HW dependencies */
#include "ee.h"
#include "hw_flash.h"

/* board dependancies */
#include "stm32wb5mm_dk_lcd.h"
#include "stm32_lcd.h"

/* Debug Part */
#include "stm_logging.h"
#include "dbg_trace.h"

/* service dependencies */
#include "app_core.h"

/* Private variables ---------------------------------------------------------*/
uint32_t persistNumWrites = 0;

/* cache in uninit RAM to store/retrieve persistent data */
union cache
{
  uint8_t  U8_data[ST_PERSIST_MAX_ALLOC_SZ];       // in bytes
  uint32_t U32_data[ST_PERSIST_MAX_ALLOC_SZ / 4U]; // in U32 words
};
__attribute__ ((section(".noinit"))) union cache cache_persistent_data;


/* Prototype Functions -------------------------------------------------------*/
void App_Log_NVM(void);

/* Persistent Functions ------------------------------------------------------*/

/**
 * @brief  Start Zigbee Network from persistent data
 * @param  zb Zigbee device object pointer
 * @retval Zigbee stack Status code
 */
enum ZbStatusCodeT App_Startup_Persist(struct ZigBeeT *zb)
{
  enum ZbStatusCodeT status = ZB_STATUS_SUCCESS;

  /* Restore persistence */
  if (App_Persist_Load())
  {
    /* Make sure the EPID is cleared, before we are allowed to restore persistence */
    uint64_t epid = 0U;
    ZbNwkSet(zb, ZB_NWK_NIB_ID_ExtendedPanId, &epid, sizeof(uint64_t));

    /* Start-up from persistence */
    APP_ZB_DBG("Restoring stack persistence");
    status = ZbStartupPersist(zb, &cache_persistent_data.U8_data[4], cache_persistent_data.U32_data[0], NULL, App_Persist_Completed_cb, zb);
  }
  else
  {
    /* Failed to restart from persistence */
    APP_ZB_DBG("No persistence data to restore");
    status = ZB_STATUS_ALLOC_FAIL;
  }

  /* Only for debug purpose, depending of persistent data, following traces
     could display bytes that are irrelevants to cluster */
  if (status == ZB_STATUS_SUCCESS) 
  {
    App_Log_NVM(); 
  }

  return status;
} /* App_Startup_Persist */


/**
 * @brief  Load persitent data
 * @param  None
 * @retval true if success, false if fail
 */
bool App_Persist_Load(void)
{
  APP_ZB_DBG("Retrieving persistent data from FLASH");
  return App_NVM_Read();
} /* App_Persist_Load */


/**
 * @brief  Save persistent data
 * @param  zb Zigbee stack instance
 * @retval true if success , false if fail
 */
bool App_Persist_Save(struct ZigBeeT *zb)
{
  uint32_t len;

  /* Clear the RAM cache before saving */
  memset(cache_persistent_data.U8_data, 0x00, ST_PERSIST_MAX_ALLOC_SZ);

  /* Call the API stack to get current persistent data */
  // len = ZbPersistGet(zb, &cache_persistent_data.U8_data[ST_PERSIST_FLASH_DATA_OFFSET], ( ST_PERSIST_MAX_ALLOC_SZ - ST_PERSIST_FLASH_DATA_OFFSET ) );
  len = ZbStateGet(zb, &cache_persistent_data.U8_data[ST_PERSIST_FLASH_DATA_OFFSET], ( ST_PERSIST_MAX_ALLOC_SZ - ST_PERSIST_FLASH_DATA_OFFSET ) );

  /* Check Length range */
  if (len == 0U)
  {
    /* If the persistence length was zero then no data available. */
    APP_ZB_DBG("No persistence data to save !");
    return false;
  }
  if (len > ST_PERSIST_MAX_ALLOC_SZ)
  {
    /* if persistence length to big to store */
    APP_ZB_DBG("Persist size too large for storage (%d)", len);
    return false;
  }

  /* Store in cache the persistent data length */
  cache_persistent_data.U32_data[0] = len;

  persistNumWrites++;
  APP_ZB_DBG("Persistence written in cache RAM (num writes = %d) len=%d",
              persistNumWrites, cache_persistent_data.U32_data[0] + ST_PERSIST_FLASH_DATA_OFFSET);

  if (!App_NVM_Write())
  {
    APP_ZB_DBG("Persistent data Error during FLASHED");
    return false;
  }
  APP_ZB_DBG("Persistent data FLASHED");
  App_Log_NVM();

  return true;
} /* App_Persist_Save */


/**
 * @brief  Delete persistent data
 * @param  None
 * @retval None
 */
void App_Persist_Delete(void)
{
  /* Clear RAM cache */
  memset(cache_persistent_data.U8_data, 0x00, ST_PERSIST_MAX_ALLOC_SZ);
  APP_ZB_DBG("Persistent Data RAM cache cleared");
  App_NVM_Erase();
  APP_ZB_DBG("FLASH ERASED");
} /* App_Persist_Delete */


/**
 * @brief  timer callback to wait end of restore cluster persistence from M0 
 * 
 * @param status of the persistence execution by M0
 * @param  arg pointer to Zigbee stack instance
 * @retval None
 */
void App_Persist_Completed_cb(enum ZbStatusCodeT status,void *arg)
{
  struct ZigBeeT *zb = arg;

  if(status == ZB_WPAN_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Persist complete callback entered with SUCCESS");
    /* Restore the application state, value based on persistence loaded */
    App_Core_Restore_State();
  }
  else
  {
    APP_ZB_DBG("Error in persist complete callback %x",status);
  }

  /* Activate back the persistent data change notifacation */
  // ZbPersistNotifyRegister(zb, App_Persist_Notify_cb, NULL);
  ZbPersistNotifyRegister(zb, NULL, NULL);
                                                         
  /* Call the callback once here to save persistence data */
  App_Persist_Notify_cb(zb, NULL); 
} /* App_Persist_Completed_cb */

/**
 * @brief  notify to save persistent data callback
 * @param  zb Zigbee device object pointer
 * @param  arg callback arg pointer
 * @retval None
 */
void App_Persist_Notify_cb(struct ZigBeeT *zb, void *arg)
{
  APP_ZB_DBG("Notification to save persistent data requested from stack");
  /* Save the persistent data */
  if (App_Persist_Save(zb) == true)
  {
    APP_ZB_DBG("Data FLASHED");
    UTIL_LCD_ClearStringLine(DK_LCD_STATUS_LINE);
    UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_STATUS_LINE), (uint8_t *)"Data FLASHED", CENTER_MODE);
    BSP_LCD_Refresh(0);
    /* Wait 2 sec before clear display */
    HAL_Delay(2000);
    UTIL_LCD_ClearStringLine(DK_LCD_STATUS_LINE);
    BSP_LCD_Refresh(0);
  }
  else
  {
    APP_ZB_DBG("Error during Data FLASHED");
    UTIL_LCD_ClearStringLine(DK_LCD_STATUS_LINE);
    UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_STATUS_LINE), (uint8_t *)"Error during Data FLASHED", CENTER_MODE);
    BSP_LCD_Refresh(0);
    /* Wait 2 sec before clear display */
    HAL_Delay(2000);
    UTIL_LCD_ClearStringLine(DK_LCD_STATUS_LINE);
    BSP_LCD_Refresh(0);
  }
} /* App_Persist_Notify_cb */


/* Exported NVM Functions ----------------------------------------------------*/
/**
 * @brief  Init the NVM
 * @param  None
 * @retval None
 */
void App_NVM_Init(void)
{
  int eeprom_init_status;

  APP_ZB_DBG("Flash starting address = %x", HW_FLASH_ADDRESS + CFG_NVM_BASE_ADDRESS);
  eeprom_init_status = EE_Init(0, HW_FLASH_ADDRESS + CFG_NVM_BASE_ADDRESS);

  if (eeprom_init_status != EE_OK)
  {
    /* format NVM since init failed */
    eeprom_init_status = EE_Init(1, HW_FLASH_ADDRESS + CFG_NVM_BASE_ADDRESS);
  }
  APP_ZB_DBG("EE_init status = %d", eeprom_init_status);

} /* App_NVM_Init */

/**
 * @brief  Read the persistent data from NVM
 * @param  None
 * @retval true if success , false if failed
 */
bool App_NVM_Read(void)
{
  uint16_t num_words = 0;
  bool status        = true;
  int ee_status      = 0;
  
  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR);

  /* Read the data length from cache */
  ee_status = EE_Read(0, ZIGBEE_DB_START_ADDR, &cache_persistent_data.U32_data[0]);
  if (ee_status != EE_OK)
  {
    APP_ZB_DBG("Read -> persistent data length not found ERASE to be done - Read Stopped");
    status = false;
  }
  /* Check length is not too big nor zero */
  else if ((cache_persistent_data.U32_data[0] == 0) ||
           (cache_persistent_data.U32_data[0] > ST_PERSIST_MAX_ALLOC_SZ))
  {
    APP_ZB_DBG("No data or too large length : %d", cache_persistent_data.U32_data[0]);
    status = false;
  }
  /* Length is within range */
  else
  {
    /* Adjust the length to be U32 aligned */
    num_words = (uint16_t)(cache_persistent_data.U32_data[0] / 4);
    if (cache_persistent_data.U32_data[0] % 4 != 0)
    {
      num_words++;
    }

    /* copy the read data from Flash to cache including length */
    for (uint16_t local_length = 1; local_length <= num_words; local_length++)
    {
      /* read data from first data in U32 unit */
      ee_status = EE_Read(0, local_length + ZIGBEE_DB_START_ADDR, &cache_persistent_data.U32_data[local_length]);
      if (ee_status != EE_OK)
      {
        APP_ZB_DBG("Read not found leaving");
        status = false;
        break;
      }
    }
  }

  HAL_FLASH_Lock();
  if (status)
  {
    APP_ZB_DBG("Read persistent data length = %d", cache_persistent_data.U32_data[0]);
  }
  return status;
} /* App_NVM_Read */


/**
 * @brief  Write the persistent data in NVM
 * @param  None
 * @retval None
 */
bool App_NVM_Write(void)
{
  int ee_status = 0;

  uint16_t num_words;
  uint16_t local_current_size;

  num_words = 1U; /* 1 words for the length */
  num_words += (uint16_t)(cache_persistent_data.U32_data[0] / 4);

  /* Adjust the length to be U32 aligned */
  if (cache_persistent_data.U32_data[0] % 4 != 0)
  {
    num_words++;
  }

  // save data in flash
  for (local_current_size = 0; local_current_size < num_words; local_current_size++)
  {
    ee_status = EE_Write(0, (uint16_t)local_current_size + ZIGBEE_DB_START_ADDR, cache_persistent_data.U32_data[local_current_size]);
    if (ee_status != EE_OK)
    {
      if (ee_status == EE_CLEAN_NEEDED) /* Shall not be there if CFG_EE_AUTO_CLEAN = 1*/
      {
        APP_ZB_DBG("CLEAN NEEDED, CLEANING");
        EE_Clean(0, 0);
      }
      else
      {
        /* Failed to write , an Erase shall be done */
        APP_ZB_DBG("App_NVM_Write failed @ %d status %d", local_current_size, ee_status);
        break;
      }
    }
  }

  if (ee_status != EE_OK)
  {
    APP_ZB_DBG("Write Stopped, need a FLASH ERASE");
    return false;
  }

  APP_ZB_DBG("Written persistent data length = %d", cache_persistent_data.U32_data[0]);
  return true;

} /* App_NVM_Write */

/**
 * @brief  Erase the NVM
 * @param  None
 * @retval None
 */
void App_NVM_Erase(void)
{
  int ee_status = 0;

  ee_status = EE_Init(1, HW_FLASH_ADDRESS + CFG_NVM_BASE_ADDRESS); /* Erase Flash */
  if (ee_status != EE_OK)
  {
    APP_ZB_DBG("Erase STOPPED, need a FLASH ERASE");
  }
} /* App_NVM_Erase */

/**
 * @brief  Simple function to see the content of NVM table
 * @param  None
 * @retval None
 */
void App_Log_NVM(void)
{
  /* read the last bytes of data where the ZCL persitent data shall be*/
  uint32_t len = cache_persistent_data.U32_data[0] + ST_PERSIST_FLASH_DATA_OFFSET;
  APP_ZB_DBG("ClusterID %02x %02x", cache_persistent_data.U8_data[len - 9], cache_persistent_data.U8_data[len - 10]);
  APP_ZB_DBG("Endpoint %02x %02x", cache_persistent_data.U8_data[len - 7], cache_persistent_data.U8_data[len - 8]);
  APP_ZB_DBG("Direction %02x", cache_persistent_data.U8_data[len - 6]);
  APP_ZB_DBG("AttrID %02x %02x", cache_persistent_data.U8_data[len - 4], cache_persistent_data.U8_data[len - 5]);
  APP_ZB_DBG("Len %02x %02x", cache_persistent_data.U8_data[len - 2], cache_persistent_data.U8_data[len - 3]);
  APP_ZB_DBG("Value %02x", cache_persistent_data.U8_data[len - 1]);
}
