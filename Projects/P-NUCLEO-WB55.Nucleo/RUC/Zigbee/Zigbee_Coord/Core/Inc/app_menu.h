/**
  ******************************************************************************
  * @file    app_menu.h
  * @author  Zigbee Application Team
  * @brief   Header for Menu file.
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
#ifndef APP_MENU_H
#define APP_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "zigbee_types.h"
#include "zigbee_interface.h"

/* Defines ------------------------------------------------------------------ */
/* Display Selection */
#define UART_DISPLAY                 0x01
#define DK_LCD_DISPLAY               0x02

/* Exported Types ------------------------------------------------------------ */
typedef struct Menu_Item_T 
{
  bool first_item_to_display;     /* To know the first item of the current-item */
  char * name;                    /* Item name to display */
  
  /* Link list foreach element in the current menu */
  struct Menu_Item_T * prev_item;
  struct Menu_Item_T * next_item;
  
  /* Manage the Tree menu */
  struct Menu_Item_T * parent;    /* Parent to callback in case of submenu */
  struct Menu_Item_T * sub_level; /* Call a submenu with new items */
  
  void (*fct)(void);              /* Action to do */
} Menu_Item_T;

/* Exported Prototypes -------------------------------------------------------*/
/* Menu creation */
bool Menu_Config(void);
Menu_Item_T * Create_Menu_Item(void);
void Add_Menu_Item(char * name,
                    Menu_Item_T * current_item, 
                    Menu_Item_T * next_item, 
                    Menu_Item_T * sub_level, 
                    void (*fct)(void) );
bool Del_Menu_Item(Menu_Item_T * item);
bool Def_Start_Menu_Item(Menu_Item_T * start_item);

/* Menu actions */
void Next_Menu_Item  (void);
void Prev_Menu_Item  (void);
void Select_Menu_Item(void);
void Exit_Menu_Item  (void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_MENU_H */