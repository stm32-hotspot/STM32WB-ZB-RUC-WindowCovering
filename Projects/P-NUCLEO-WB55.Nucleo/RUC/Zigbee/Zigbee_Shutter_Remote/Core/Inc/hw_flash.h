/**
  ******************************************************************************
 * @file    hw_flash.h
 * @author  MCD Application Team
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
#ifndef __HW_FLASH_H
#define __HW_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

  /* Flash address */
#define HW_FLASH_ADDRESS                FLASH_BASE

/* Flash page size in bytes */
#define HW_FLASH_PAGE_SIZE              FLASH_PAGE_SIZE

/* Flash width in bytes */
#define HW_FLASH_WIDTH                  8             

/* Return values definition */
enum
{
  HW_OK     = 0,
  HW_BUSY   = 1
};

int HW_FLASH_Write(uint32_t address, uint64_t data);
int HW_FLASH_Erase(uint32_t page, uint16_t n, int interrupt);

#ifdef __cplusplus
}
#endif

#endif /*__HW_FLASH_H */
