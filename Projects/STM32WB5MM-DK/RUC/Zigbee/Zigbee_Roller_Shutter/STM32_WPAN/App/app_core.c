/**
  ******************************************************************************
  * @file    app_core.c
  * @author  Zigbee Application Team
  * @brief   Application Core to centralize all functionnalities
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
#include "app_core.h"

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "app_entry.h"
#include "app_zigbee.h"
#include "shci.h"
#include "stm32_seq.h"
#include "stm32wbxx_core_interface_def.h"

#include "zigbee_types.h"
#include "zigbee_interface.h"

/* board dependancies */
#include "stm32wb5mm_dk_lcd.h"
#include "stm32_lcd.h"

/* Debug Part */
#include <assert.h>
#include "stm_logging.h"
#include "dbg_trace.h"

/* service dependencies */
#include "app_zigbee.h"
#include "app_nvm.h"
#include "app_menu.h"
#include "app_light_cfg.h"
#include "app_roller_shutter_cfg.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  Normal_mode,
  Demo_mode,
} Menu_Mode_Type_T;

/* Private Variables----------------------------------------------------------*/
static Menu_Mode_Type_T menu_mode = Normal_mode;

/* External variables ------------------------------------------------------- */
extern App_Zb_Info_T app_zb_info;
extern uint8_t       display_type;

/* Application Variable-------------------------------------------------------*/
bool id_mode_on = false;

/* Private functions prototypes-----------------------------------------------*/
/* Buttons/Touchkey management for the application */
static void App_SW1_Action(void);
static void App_SW2_Action(void);
static void App_Core_UpdateButtonState(Button_TypeDef button, int isPressed);

/* Informations functions */
static void App_Core_Name_Disp            (void);
/* DK LCD Clean */
static void App_Core_Display_Clean_Status (void);

/* Others Action */
static void App_Core_Leave_cb (struct ZbNlmeLeaveConfT *conf, void *arg);

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief Core of the Application to manage all services to use
 * 
 */
void App_Core_Init(void)
{
  APP_ZB_DBG("Initialisation");

  App_Core_Name_Disp();

  App_Zigbee_Init();

  /* Task associated with button Action */
  UTIL_SEQ_RegTask(1U << CFG_TASK_BUTTON_SW1, UTIL_SEQ_RFU, App_SW1_Action);
  UTIL_SEQ_RegTask(1U << CFG_TASK_BUTTON_SW2, UTIL_SEQ_RFU, App_SW2_Action);

  /* prepare task to clean display */
  UTIL_SEQ_RegTask(1U << CFG_TASK_LCD_CLEAN_STATUS, UTIL_SEQ_RFU, App_Core_Display_Clean_Status);

  /* Initialize Zigbee stack layers */
  App_Zigbee_StackLayersInit();

  /* init Tx power to the default value */
  App_Zigbee_TxPwr_Disp();

  /* Display informations after Join */
  if (app_zb_info.join_status == ZB_STATUS_SUCCESS)
    App_Zigbee_Channel_Disp();
  
  /* Menu Config */
  if (Menu_Config() == 0)
  {
    APP_ZB_DBG("Error : Menu Config");
    UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_STATUS_LINE), (uint8_t *)"Error : Menu Config", CENTER_MODE);
    BSP_LCD_Refresh(0);
  }
} /* App_Core_Init */

/**
 * @brief Configure the Device End points according to the clusters used
 *        Call during the StackLayerInit
 * 
 */
void App_Core_ConfigEndpoints(void)
{
  APP_ZB_DBG("Configure the Endpoints for the device");
  App_Roller_Shutter_Cfg_Endpoint(app_zb_info.zb);
  App_Light_Cfg_Endpoint         (app_zb_info.zb);
} /* App_Core_ConfigEndpoints */


/**
 * @brief  Restore the application state as read cluster attribute after a startup from persistence
 * @param  None
 * @retval None
 */
void App_Core_Restore_State(void)
{
  APP_ZB_DBG("Restore Cluster informations for the application");
  App_Roller_Shutter_Restore_State();
  App_Light_Restore_State         ();
  App_Zigbee_Bind_Disp();
} /* App_Core_Restore_State */


/* Informations functions -------------------------------------------------- */
/**
 * @brief  LCD DK Application name initialisation
 * 
 * @param  None
 * @retval None
 */
static void App_Core_Name_Disp(void)
{

  /* Display Application information header */
  APP_ZB_DBG("RUC Zigbee Roller Shutter Demo");
  /* Text Feature */
  BSP_LCD_Clear(0,SSD1315_COLOR_BLACK);
  UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_APP_NAME_LINE), (uint8_t *)"Zb Shutter Demo", CENTER_MODE);
  BSP_LCD_Refresh(0);
} /* App_Core_Name_Disp */

/**
 * @brief  Clean LCD display at the end of the status display
 * 
 * @param  None
 * @retval None
 */
static void App_Core_Display_Clean_Status(void)
{
  UTIL_LCD_ClearStringLine(DK_LCD_STATUS_LINE);
  BSP_LCD_Refresh(0);
} /* App_Core_Display_Clean_Status */

/**
 * @brief Clean and refresh the global informations on the application ongoing
 * To call from the Menu when the user wants to retrevie the base informations
 * 
 */
void App_Core_Infos_Disp(void)
{
  APP_ZB_DBG("**********************************************************");
  App_Core_Name_Disp();
  App_Zigbee_Channel_Disp();
  App_Zigbee_TxPwr_Disp();
  App_Zigbee_Check_Firmware_Info();
  App_Zigbee_All_Address_Disp();
  App_Zigbee_Bind_Disp();
  APP_ZB_DBG("**********************************************************");
  Menu_Config();
} /* App_Core_Infos_Disp */


/* Network Actions ---------------------------------------------------------- */
/**
 * @brief Launches the Network joining action when the user ready to add the device at network
 * could be called by menu action
 * 
 */
void App_Core_Ntw_Join(void)
{
  APP_ZB_DBG("Launching Network Join");
  LED_Set_rgb(PWM_LED_GSDATA_OFF, PWM_LED_GSDATA_OFF, PWM_LED_GSDATA_47_0);
  UTIL_LCD_ClearStringLine(DK_LCD_STATUS_LINE);
  UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_STATUS_LINE), (uint8_t *)"Network Join", CENTER_MODE);
  BSP_LCD_Refresh(0);

  /* If Network joining was not successful reschedule the current task to retry the process */
  while (app_zb_info.join_status != ZB_STATUS_SUCCESS)
  {
    /* No Rejoin Network from persistence so decide todo it */
    UTIL_SEQ_SetTask(1U << CFG_TASK_ZIGBEE_NETWORK_JOIN, CFG_SCH_PRIO_0);
    UTIL_SEQ_WaitEvt(EVENT_ZIGBEE_NETWORK_JOIN);
  }
  
  /* Indicates successful join*/
  LED_Off();
  HAL_Delay(300);
  LED_Set_rgb(PWM_LED_GSDATA_OFF, PWM_LED_GSDATA_47_0, PWM_LED_GSDATA_OFF);
  HAL_Delay(300);
  LED_Set_rgb(PWM_LED_GSDATA_OFF, PWM_LED_GSDATA_OFF, PWM_LED_GSDATA_47_0);
  HAL_Delay(300);
  LED_Set_rgb(PWM_LED_GSDATA_OFF, PWM_LED_GSDATA_47_0, PWM_LED_GSDATA_OFF);
  HAL_Delay(300);
  LED_Set_rgb(PWM_LED_GSDATA_OFF, PWM_LED_GSDATA_OFF, PWM_LED_GSDATA_47_0);
  HAL_Delay(300);
  LED_Off();

  /* Display informations after Join */
  App_Zigbee_Channel_Disp();
  UTIL_SEQ_SetTask(1U << CFG_TASK_LCD_CLEAN_STATUS, CFG_SCH_PRIO_1);
} /* App_Core_Ntw_Join */


/* Actions from Menu ------------------------------------------------------- */
/**
 * @brief  Leave the Zigbbe network, clean the flash and reboot of the chip
 * 
 */
void App_Core_Factory_Reset(void)
{
  char LCD_Text[32];

  sprintf(LCD_Text, "FACTORY RESET");
  APP_ZB_DBG("%s", LCD_Text);
  /* Text Feature */
  UTIL_LCD_ClearStringLine(DK_LCD_STATUS_LINE);
  UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_STATUS_LINE), (uint8_t *)LCD_Text, CENTER_MODE);
  BSP_LCD_Refresh(0);
  // use ZbLeaveReq before to send information to the network before leave it
  if (app_zb_info.join_status == ZB_STATUS_SUCCESS)
  {
    App_Zigbee_Unbind_All();
    ZbLeaveReq(app_zb_info.zb, &App_Core_Leave_cb, NULL);
  }
  App_Persist_Delete();
  /* Wait 2 sec before clear display */
  HAL_Delay(2000);
  UTIL_SEQ_SetTask(1U << CFG_TASK_LCD_CLEAN_STATUS, CFG_SCH_PRIO_1);
  NVIC_SystemReset();
} /* App_Core_Factory_Reset */

/**
 * @brief Call back after perform an NLME-LEAVE.request
 * 
 * @param conf 
 * @param arg unused
 */
static void App_Core_Leave_cb(struct ZbNlmeLeaveConfT *conf, void *arg)
{
  /* check if device left correctly/not the network */
  if (conf->status != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Error %x during leave the network", conf->status);
  }
}


/* Buttons management for the application ------------------------------------*/
/**
 * @brief Wrapper to manage the short/long press
 * 
 */
static void App_SW1_Action(void)
{
  App_Core_UpdateButtonState(BUTTON_USER1, BSP_PB_GetState(BUTTON_USER1) == BUTTON_PRESSED);
  return;
}
static void App_SW2_Action(void)
{
  App_Core_UpdateButtonState(BUTTON_USER2, BSP_PB_GetState(BUTTON_USER2) == BUTTON_PRESSED);
  return;
}

/**
 * @brief associate the buttons and the action to call/launch inside the application
 * Could be pass in Demo mode during Salon event or in Normal Mode 
 * Normal mode : give the access to the Zigbee Network functionnalities and others applications
 * Demo mode   : mask menu on LCD&UART and unactive the USER1 & USER2 buttons
 * 
 * @param button using
 * @param isPressed state
 */
static void App_Core_UpdateButtonState(Button_TypeDef button, int isPressed)
{
  uint32_t t0 = 0,press_delay = 1;
  t0 = HAL_GetTick(); /* button press timing */
  while(BSP_PB_GetState(button) == BUTTON_PRESSED && press_delay <= MAX_PRESS_DELAY)
    press_delay = HAL_GetTick() - t0;

  // Switch to Normal/Demo mode
  if ( (BSP_PB_GetState(BUTTON_USER1) == BUTTON_PRESSED) && (BSP_PB_GetState(BUTTON_USER2) == BUTTON_PRESSED) )
  {
    while(BSP_PB_GetState(button) == BUTTON_PRESSED && press_delay <= LONG_PRESS_DELAY )
      press_delay = HAL_GetTick() - t0;
    
    // No action due to press too short
    if (press_delay < MIDDLE_PRESS_DELAY )
      return;
    
    // Switch display to Normal/Demo mode
    if (menu_mode != Demo_mode)
    {
      menu_mode = Demo_mode;
      display_type = NULL;
      APP_ZB_DBG("Menu OFF");      
      // Clear all lines to pass in Demo mode
      UTIL_LCD_Clear(SSD1315_COLOR_BLACK);
      App_Core_Name_Disp();
    }
    // Pass LCD in Debug Mode with UART/LCD menu access
    else
    {
      menu_mode = Normal_mode;
      display_type = (UART_DISPLAY | DK_LCD_DISPLAY);
      APP_ZB_DBG("Menu ON");
      // App_Persist_Notify_cb(app_zb_info.zb, NULL);
    }
    while( (BSP_PB_GetState(BUTTON_USER1) == BUTTON_PRESSED) || (BSP_PB_GetState(BUTTON_USER2) == BUTTON_PRESSED) )
    {
      HAL_Delay(500);
    };
  }
  
  // Check if Normal Mode for debug so display the menu to work on the Network
  if (menu_mode == Normal_mode)
  {
    /* Manage Push button time by each button */
    /* From Long time to short time*/
    switch (button)
    {
      case BUTTON_USER1:
        if (press_delay > MIDDLE_PRESS_DELAY)
        {
          /* exit current submenu and Up to previous Menu */
          Exit_Menu_Item();
        }
        else if (press_delay > DEBOUNCE_DELAY)
        {
          /* Change menu selection to left */
          Prev_Menu_Item();       
        }
        break;

      case BUTTON_USER2:
        if (press_delay > MIDDLE_PRESS_DELAY)
        {
          /* execute action or enter sub-menu */
          Select_Menu_Item();
        }
        else if (press_delay > DEBOUNCE_DELAY)
        {
          /* Change menu selection to right */
          Next_Menu_Item();       
        }
        break;

      default:
        APP_ZB_DBG("ERROR : Button Unknow");
        break;
    }
  }
} /* App_Core_UpdateButtonState */

