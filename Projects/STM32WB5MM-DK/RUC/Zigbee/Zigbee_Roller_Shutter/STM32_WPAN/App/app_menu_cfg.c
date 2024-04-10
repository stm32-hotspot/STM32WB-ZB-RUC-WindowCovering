/**
******************************************************************************
* @file    app_menu_cfg.c
* @author  Zigbee Application Team
* @brief   Configuration interface of menu for application
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
#include "app_menu.h"
#include "app_nvm.h"
#include "app_zigbee.h"
#include "app_core.h"
#include "app_roller_shutter_cfg.h"
#include "app_light_cfg.h"

/* External variables ------------------------------------------------------- */
extern uint8_t           display_type;
extern App_Zb_Info_T     app_zb_info;

/* Private functions prototypes-----------------------------------------------*/
/* Menu app Config */
static void app_launch_nvm(void);


/* Functions Definition -------------------------------------------- */

/**
 * @brief  Configure the Menu tree
 * @param  display_select to choose where the Menu appears
 * @retval state
 */
bool Menu_Config(void)
{
  /* Choose where to display the menu */
  display_type = (UART_DISPLAY | DK_LCD_DISPLAY);

  /* Menu item instances -----------------------------------------------------*/
  // 1st level of menu
  Menu_Item_T * menu_reset                = Create_Menu_Item();
  Menu_Item_T * menu_info                 = Create_Menu_Item();

  // Network Menu
  Menu_Item_T * menu_ntw                  = Create_Menu_Item();
  Menu_Item_T * menu_id_mode              = Create_Menu_Item();
  Menu_Item_T * menu_ntw_join             = Create_Menu_Item();
  Menu_Item_T * menu_permit_join          = Create_Menu_Item();
  Menu_Item_T * menu_ntw_bind_tab         = Create_Menu_Item();
  Menu_Item_T * menu_ntw_unbind_all       = Create_Menu_Item();
  Menu_Item_T * menu_ntw_txpwr_disp       = Create_Menu_Item();
  Menu_Item_T * menu_ntw_txpwr_up         = Create_Menu_Item();
  Menu_Item_T * menu_ntw_txpwr_down       = Create_Menu_Item();
  Menu_Item_T * menu_ntw_NVM              = Create_Menu_Item();
  
  // Shutter conf Menu
  Menu_Item_T * menu_shutter_cfg          = Create_Menu_Item();
  Menu_Item_T * menu_shutter_id_mode      = Create_Menu_Item();  
  Menu_Item_T * menu_shutter_findbind     = Create_Menu_Item();
  Menu_Item_T * menu_timer_motorup_up     = Create_Menu_Item();
  Menu_Item_T * menu_timer_motorup_down   = Create_Menu_Item();  
  Menu_Item_T * menu_timer_motordown_up   = Create_Menu_Item();
  Menu_Item_T * menu_timer_motordown_down = Create_Menu_Item(); 
  Menu_Item_T * menu_motor_speed_up       = Create_Menu_Item();
  Menu_Item_T * menu_motor_speed_down     = Create_Menu_Item();
  Menu_Item_T * menu_adc_treshold_up      = Create_Menu_Item();
  Menu_Item_T * menu_adc_treshold_down    = Create_Menu_Item(); 
  // Shutter cmd Menu
  Menu_Item_T * menu_shutter_cmd          = Create_Menu_Item();
  Menu_Item_T * menu_shutter_up           = Create_Menu_Item();
  Menu_Item_T * menu_shutter_down         = Create_Menu_Item();
  Menu_Item_T * menu_shutter_stop         = Create_Menu_Item(); 
  
  // Light Menu
  Menu_Item_T * menu_light_cfg            = Create_Menu_Item();
  Menu_Item_T * menu_light_id_mode        = Create_Menu_Item();
  Menu_Item_T * menu_light_led_toggle     = Create_Menu_Item();
  Menu_Item_T * menu_light_led_lvl_up     = Create_Menu_Item();
  Menu_Item_T * menu_light_led_lvl_down   = Create_Menu_Item();
 
  
  /* Menu link --------------------------------------------------------------*/
  // Main menu --------|  Menu name     | Current Item     | Next Item        | Sub-Menu             | Action to launch        |
  Add_Menu_Item((char *) "Network"      , menu_ntw         , menu_shutter_cfg , menu_ntw_join        , NULL);
  Add_Menu_Item((char *) "Window Cfg"   , menu_shutter_cfg , menu_shutter_cmd , menu_shutter_id_mode , NULL);
  Add_Menu_Item((char *) "Window Cmd"   , menu_shutter_cmd , menu_light_cfg   , menu_shutter_up      , NULL);
  Add_Menu_Item((char *) "Light"        , menu_light_cfg   , menu_reset       , menu_light_id_mode   , NULL);
  Add_Menu_Item((char *) "Factory Reset", menu_reset       , menu_info        , NULL                 , &App_Core_Factory_Reset);
  Add_Menu_Item((char *) "Global Infos" , menu_info        , menu_ntw         , NULL                 , &App_Core_Infos_Disp);

  // Network Menu, 
  Add_Menu_Item((char *) "Join Network" , menu_ntw_join      , menu_permit_join   , NULL             , &App_Core_Ntw_Join);
  Add_Menu_Item((char *) "Permit_Join"  , menu_permit_join   , menu_ntw_bind_tab  , NULL             , &App_Zigbee_Permit_Join);
  Add_Menu_Item((char *) "Bind Table"   , menu_ntw_bind_tab  , menu_ntw_txpwr_disp, NULL             , &App_Zigbee_Bind_Disp);
  Add_Menu_Item((char *) "Tx Power Disp", menu_ntw_txpwr_disp, menu_ntw_txpwr_up  , NULL             , &App_Zigbee_TxPwr_Disp);
  Add_Menu_Item((char *) "Tx Power +"   , menu_ntw_txpwr_up  , menu_ntw_txpwr_down, NULL             , &App_Zigbee_TxPwr_Up);
  Add_Menu_Item((char *) "Tx Power -"   , menu_ntw_txpwr_down, menu_ntw_NVM       , NULL             , &App_Zigbee_TxPwr_Down);
  Add_Menu_Item((char *) "Launch NVM"   , menu_ntw_NVM       , menu_ntw_join      , NULL             , &app_launch_nvm);

  // Shutter conf Menu
  Add_Menu_Item((char *) "ID Mode"       , menu_shutter_id_mode     , menu_shutter_findbind    , NULL, &App_Roller_Shutter_IdentifyMode);  
  Add_Menu_Item((char *) "Find & Bind"   , menu_shutter_findbind    , menu_timer_motorup_up    , NULL, &App_Roller_Shutter_FindBind);   
  Add_Menu_Item((char *) "Timer Up +"    , menu_timer_motorup_up    , menu_timer_motorup_down  , NULL, &App_Roller_Shutter_timer_motorup_up);
  Add_Menu_Item((char *) "Timer Up -"    , menu_timer_motorup_down  , menu_timer_motordown_up  , NULL, &App_Roller_Shutter_timer_motorup_down);
  Add_Menu_Item((char *) "Timer Down +"  , menu_timer_motordown_up  , menu_timer_motordown_down, NULL, &App_Roller_Shutter_timer_motordown_up);
  Add_Menu_Item((char *) "Timer Down -"  , menu_timer_motordown_down, menu_motor_speed_up      , NULL, &App_Roller_Shutter_timer_motordown_down);
  Add_Menu_Item((char *) "Motor speed +" , menu_motor_speed_up      , menu_motor_speed_down    , NULL, &App_Roller_Shutter_motor_speed_up);
  Add_Menu_Item((char *) "Motor speed -" , menu_motor_speed_down    , menu_adc_treshold_up     , NULL, &App_Roller_Shutter_motor_speed_down);
  Add_Menu_Item((char *) "ADC Treshold +", menu_adc_treshold_up     , menu_adc_treshold_down   , NULL, &App_Roller_Shutter_adc_treshold_up);
  Add_Menu_Item((char *) "ADC Treshold -", menu_adc_treshold_down   , menu_shutter_id_mode     , NULL, &App_Roller_Shutter_adc_treshold_down);
  // Shutter cmd Menu
  Add_Menu_Item((char *) "Shutter Up"    , menu_shutter_up          , menu_shutter_stop        , NULL, &App_Roller_Shutter_Up);
  Add_Menu_Item((char *) "Shutter Stop"  , menu_shutter_stop        , menu_shutter_down        , NULL, &App_Roller_Shutter_Stop);
  Add_Menu_Item((char *) "Shutter Down"  , menu_shutter_down        , menu_shutter_up          , NULL, &App_Roller_Shutter_Down);
  
  // Light Menu
  Add_Menu_Item((char *) "ID Mode"       , menu_light_id_mode       , menu_light_led_toggle    , NULL, &App_Light_IdentifyMode);
  Add_Menu_Item((char *) "Led Toggle"    , menu_light_led_toggle    , menu_light_led_lvl_up    , NULL, &App_Light_Toggle);
  Add_Menu_Item((char *) "Led lvl +"     , menu_light_led_lvl_up    , menu_light_led_lvl_down  , NULL, &App_Light_Level_Up);
  Add_Menu_Item((char *) "Led lvl -"     , menu_light_led_lvl_down  , menu_light_id_mode       , NULL, &App_Light_Level_Down);
  
  return Def_Start_Menu_Item(menu_ntw);
} /* Menu_config */

/* User menu config function ---------------------------------------------------*/
static void app_launch_nvm(void)
{
  // TODO remove it when the NVM works correctly
  App_Persist_Notify_cb(app_zb_info.zb, NULL);
  HAL_Delay(500);
}


