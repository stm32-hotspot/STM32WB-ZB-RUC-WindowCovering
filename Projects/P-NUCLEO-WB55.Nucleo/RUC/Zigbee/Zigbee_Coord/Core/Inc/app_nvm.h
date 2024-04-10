/**
  ******************************************************************************
  * @file    app_nvm.h
  * @author  Zigbee Application Team
  * @brief   Header for NVM application file.
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
#ifndef APP_NVM_H
#define APP_NVM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "zigbee_interface.h"

/* Defines -----------------------------------------------------------*/

  /* 
    CFG_EE_BANK0_SIZE is the size allocated for the EE bank0 it should be
    the considered as the max Flash size for all computation and <= of the
    allocated size within the scatterfile in bytes
    
    CFG_NVM_BASE_ADDRESS : offset to add to the base flash address to get the 
    beginning of the NVM (shall be within  allocation range of scatterfile)
  
    ST_PERSIST_MAX_ALLOC_SZ : max size of the RAM cache in bytes
                              either an abitrary choice or the CFG_NVM_MAX_SIZE

    ST_PERSIST_FLASH_DATA_OFFSET : offset in bytes of zigbee data
    (U8[4] for length  - 1st data[]...)

    CFG_NB_OF_PAGE : Number of page of flash to use
    
    CFG_NVM_MAX_SIZE : Max allocable size in byte for NVM
                    Flash size/8 * (number of element by page in byte)

    ZIGBEE_DB_START_ADDR: beginning of zigbee NVM

    CFG_EE_AUTO_CLEAN : Clean the flash automatically when needed
  */ 
#define CFG_NB_OF_PAGE                          (16U)
#define CFG_EE_BANK0_SIZE                       (CFG_NB_OF_PAGE * HW_FLASH_PAGE_SIZE) 
#define CFG_NVM_BASE_ADDRESS                    ( 0x70000U )
#define CFG_EE_BANK0_MAX_NB                     (1000U)                  // In U32 words
#define ST_PERSIST_MAX_ALLOC_SZ                 (4U*CFG_EE_BANK0_MAX_NB) // Max data in bytes
#define ST_PERSIST_FLASH_DATA_OFFSET            (4U)
#define ZIGBEE_DB_START_ADDR                    (0U)
#define CFG_EE_AUTO_CLEAN                       (1U)

/* Exported Persistent Prototypes --------------------------------------------*/
enum ZbStatusCodeT App_Startup_Persist(struct ZigBeeT *zb);
bool App_Persist_Load        (void);
bool App_Persist_Save        (struct ZigBeeT *zb);
void App_Persist_Delete      (void);
void App_Persist_Completed_cb(enum ZbStatusCodeT status, void *arg);
void App_Persist_Notify_cb   (struct ZigBeeT *zb, void *arg);

/* Exported NVM Prototypes ---------------------------------------------------*/
void App_NVM_Init (void);
bool App_NVM_Read (void);
bool App_NVM_Write(void);
void App_NVM_Erase(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_NVM_H */