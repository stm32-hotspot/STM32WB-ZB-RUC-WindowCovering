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

#include "app_zigbee.h"
#include "app_core.h"
#include "app_roller_shutter_remote_cfg.h"

/* External variables ------------------------------------------------------- */
extern uint8_t          display_type;

/* Functions Definition ----------------------------------------------------- */

/**
 * @brief  Configure the Menu tree
 * @param  display_select to choose where the Menu appears
 * @retval state
 */
bool Menu_Config(void)
{
  /* Choose where to display the menu */
  display_type = UART_DISPLAY;

  /* Menu item instances -----------------------------------------------------*/
  // 1st level of menu
  Menu_Item_T * menu_reset= Create_Menu_Item();
  Menu_Item_T * menu_info = Create_Menu_Item();

  // Network Menu
  Menu_Item_T * menu_ntw             = Create_Menu_Item();
  Menu_Item_T * menu_ntw_join        = Create_Menu_Item();
  Menu_Item_T * menu_ntw_findbind    = Create_Menu_Item();
  Menu_Item_T * menu_ntw_bind_tab    = Create_Menu_Item();
  Menu_Item_T * menu_ntw_txpwr_disp  = Create_Menu_Item();
  Menu_Item_T * menu_ntw_txpwr_up    = Create_Menu_Item();
  Menu_Item_T * menu_ntw_txpwr_down  = Create_Menu_Item();
  
  // Roller Shutter Menu
  Menu_Item_T * menu_shutter         = Create_Menu_Item();  
  Menu_Item_T * menu_shutter_up      = Create_Menu_Item();  
  Menu_Item_T * menu_shutter_stop    = Create_Menu_Item();  
  Menu_Item_T * menu_shutter_down    = Create_Menu_Item();  
  
  /* Menu link ---------------------------------------------------------------*/
  // Main menu --------|  Menu name     | Current Item       | Next Item          | Sub-Menu         | Action to launch        |
  Add_Menu_Item((char *) "Network"      , menu_ntw           , menu_shutter       , menu_ntw_join    , NULL                    );
  Add_Menu_Item((char *) "Window"       , menu_shutter       , menu_reset         , menu_shutter_up  , NULL                    );
  Add_Menu_Item((char *) "Factory Reset", menu_reset         , menu_info          , NULL             , &App_Core_Factory_Reset );
  Add_Menu_Item((char *) "Global Infos" , menu_info          , menu_ntw           , NULL             , &App_Core_Infos_Disp    );

  // Network Menu, 
  Add_Menu_Item((char *) "Join Network" , menu_ntw_join      , menu_ntw_findbind  , NULL             , &App_Core_Ntw_Join      );
  Add_Menu_Item((char *) "Find & Bind"  , menu_ntw_findbind  , menu_ntw_bind_tab  , NULL             , &App_Roller_Shutter_Remote_FindBind);
  Add_Menu_Item((char *) "Bind Table"   , menu_ntw_bind_tab  , menu_ntw_txpwr_disp, NULL             , &App_Roller_Shutter_Remote_Bind_Disp);
  Add_Menu_Item((char *) "Tx Power Disp", menu_ntw_txpwr_disp, menu_ntw_txpwr_up  , NULL             , &App_Zigbee_TxPwr_Disp  );
  Add_Menu_Item((char *) "Tx Power +"   , menu_ntw_txpwr_up  , menu_ntw_txpwr_down, NULL             , &App_Zigbee_TxPwr_Up    );
  Add_Menu_Item((char *) "Tx Power -"   , menu_ntw_txpwr_down, menu_ntw_join      , NULL             , &App_Zigbee_TxPwr_Down  );

  // Window Menu, 
  Add_Menu_Item((char *) "Shutter up"   , menu_shutter_up   , menu_shutter_stop   , NULL             , &App_Roller_Shutter_Remote_Move_Up  );
  Add_Menu_Item((char *) "Shutter stop" , menu_shutter_stop , menu_shutter_down   , NULL             , &App_Roller_Shutter_Remote_Move_Stop);
  Add_Menu_Item((char *) "Shutter down" , menu_shutter_down , menu_shutter_up     , NULL             , &App_Roller_Shutter_Remote_Move_Down);

  return Def_Start_Menu_Item(menu_ntw);
} /* Menu_config */

