/**
  ******************************************************************************
  * @file    app_core.h
  * @author  Zigbee Application Team
  * @brief   Header for Core application file.
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
#ifndef APP_CORE_H
#define APP_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Defines -----------------------------------------------------------*/
/* Buttons timer */
#define BUTTON_PRESSED                 GPIO_PIN_RESET
#define BUTTON_RELEASED                GPIO_PIN_SET
#define DEBOUNCE_PRESS                 10U
#define DEBOUNCE_DELAY                 (DEBOUNCE_PRESS * HW_TS_SERVER_1ms_NB_TICKS)
#define MIDDLE_PRESS                   200U
#define MIDDLE_PRESS_DELAY             (MIDDLE_PRESS   * HW_TS_SERVER_1ms_NB_TICKS)
#define LONG_PRESS                     3U
#define LONG_PRESS_DELAY               (LONG_PRESS     * HW_TS_SERVER_1S_NB_TICKS)
#define MAX_PRESS_DELAY                MIDDLE_PRESS_DELAY


#define LED_TOGGLE_DELAY               200U
#define HW_TS_LED_TOGGLE_DELAY         (LED_TOGGLE_DELAY * HW_TS_SERVER_1ms_NB_TICKS)  /**< 0.5s */
#define PERMIT_JOIN_DELAY              60U
#define HW_TS_PERMITJOIN_DELAY         (PERMIT_JOIN_DELAY * HW_TS_SERVER_1S_NB_TICKS)

#define HW_TS_CLEAN_NVM_TIMEOUT        1000U
#define HW_TS_TOUCHKEY_SAMPLE          50U

/* Application definition to display some informations */
#define DK_LCD_APP_NAME_LINE 0
#define DK_LCD_CHANNEL_LINE  1
#define DK_LCD_INFO_LINE     2
#define DK_LCD_MENU_LINE     3
#define DK_LCD_STATUS_LINE   4

#define DK_LCD_SHUTTER_DISP  2
#define DK_LCD_LIGHT_DISP    4

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/* Application Init functions */
void App_Core_Init           (void);
void App_Core_ConfigEndpoints(void);
void App_Core_Restore_State  (void);

/* Action from menu */
void App_Core_Infos_Disp     (void);
void App_Core_Ntw_Join       (void);
void App_Core_Factory_Reset  (void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_CORE_H */