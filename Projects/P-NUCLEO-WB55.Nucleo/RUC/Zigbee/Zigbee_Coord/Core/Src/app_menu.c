/**
******************************************************************************
* @file    app_menu.c
* @author  Zigbee Application Team
* @brief   Application interface for menu
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

#include "stm_logging.h"
#include "dbg_trace.h"

/* External variables ------------------------------------------------------- */
uint8_t display_type;

/* Private defines ---------------------------------------------------------- */
#define MENU_REFRESH_DELAY           2
#define HW_TS_MENU_REFRESH_DELAY     (MENU_REFRESH_DELAY * HW_TS_SERVER_1S_NB_TICKS)

/* Private variables -------------------------------------------------------- */
static Menu_Item_T * current_menu;
static int           nb_of_menu_item;
static uint8_t       TS_ID_REFRESH_MENU_DISP;

/* Private functions prototypes-----------------------------------------------*/
/* Menu Methode */
static bool Check_Menu     (Menu_Item_T * menu_item);
static void Calc_Item_Nb   (void);
static void Print_Menu     (void);
static void Clear_UART_Line(void);


/* Exported Functions Definition -------------------------------------------- */

// Navigation fonctions from your application by buttons
/**
 * @brief  Get next item from menu
 * @param  None
 * @retval None
 */
void Next_Menu_Item(void)
{
  current_menu = current_menu->next_item;
  /* Display New Menu selection */
  Print_Menu();      
} /* Next_Menu_Item */

/**
 * @brief  Get previous item from menu
 * @param  None
 * @retval None
 */
void Prev_Menu_Item(void)
{
  current_menu = current_menu->prev_item;
  /* Display New Menu selection */
  Print_Menu();   
} /* Prev_Menu_Item */

/**
 * @brief  Do the action associated to the item
 * @param  arg pointer to launch the item
 * @retval None
 */
void Select_Menu_Item(void)
{
  /* If this item get an sub menu, change current menu selection */
  if (current_menu->sub_level != NULL)
  {
    if (Check_Menu(current_menu->sub_level))
    {
      current_menu = current_menu->sub_level;
      /* Calcule the new number of item in this menu */
      Calc_Item_Nb();
      Clear_UART_Line();        /* Clear Terminal line*/
      Print_Menu();      
    }
    else
    {
      Clear_UART_Line();        /* Clear Terminal line*/
      APP_ZB_DBG("Impossible to go in sub menu : Check Error");
    }
  }
  /* Launch the associated function */
  else if (current_menu->fct != NULL)
  {
    Clear_UART_Line();        /* Clear Terminal line*/
    current_menu->fct();
  }
} /* Select_Menu_Item */

/**
 * @brief  Exit current menu to switch to main menu
 * @param  None
 * @retval None
 */
void Exit_Menu_Item(void)
{
  if (current_menu->parent != NULL)
  {
    current_menu = current_menu->parent;
    Calc_Item_Nb();
    Clear_UART_Line();        /* Clear Terminal line*/
    Print_Menu();
  }
  else
  {
    Clear_UART_Line();        /* Clear Terminal line*/
    APP_ZB_DBG("No main menu");
  }
} /* Exit_Menu_Item */

// Configuration fonctions for your menu tree
/**
 * @brief  Create the instance of menu item
 * @retval pointer to the instance item
 */
Menu_Item_T * Create_Menu_Item(void)
{
  Menu_Item_T * menu_item = (Menu_Item_T *) malloc(sizeof(Menu_Item_T));
  
  menu_item->first_item_to_display = false;
  menu_item->name                  = NULL;
  menu_item->prev_item             = NULL;
  menu_item->next_item             = NULL;
  menu_item->parent                = NULL;
  menu_item->sub_level             = NULL;
  menu_item->fct                   = NULL;
  
  return menu_item;
} /* Create_Menu_Item */
  
/**
 * @brief  Add an item to the current menu, repat it for each item to create the full menu
 * @param  name of current item of the menu
 * @param  current_item to add to menu
 * @param  next_item of menu
 * @param  sub_level of tree menu 
 * @param  fct to launch for action associated
 * @retval None
 */
void  Add_Menu_Item(char* name, Menu_Item_T* current_item, Menu_Item_T * next_item,
                    Menu_Item_T* sub_level, void (*fct)(void) )
{
  /* Global check to init correctly the item and avoid mismatch */
  if ((sub_level != NULL) && (fct != NULL))
  {
    APP_ZB_DBG("Warning : [%s] menu item has both function and sub menu config. Only one is supported", name);
  }
  else if ((sub_level == NULL) && (fct == NULL))
  {
    APP_ZB_DBG("Warning : [%s] menu item hasn't neither function, nor sub menu config. One is needed", name);
  }
  else
  {
    /* Init instance menu item */
    current_item->name      = name;
    current_item->next_item = next_item;
    next_item->prev_item    = current_item;   
    current_item->fct       = fct;
    
    /* check if it is the top of the tree */
    if (current_item->parent != NULL)
    {
      next_item->parent = current_item->parent;      
    }

    /* check if it is a sub-level ot the current_item */
    if (sub_level != NULL)
    {
      current_item->sub_level          = sub_level;
      /* auto init some parts of the sub-level */
      sub_level->parent                = current_item;
      sub_level->first_item_to_display = true;      
    }    
  }
} /*  Add_Menu_Item */

/**
 * @brief  Delete an item from menu
 * @param  item to delete
 * @retval state
 */
bool Del_Menu_Item(Menu_Item_T * item)
{
  /* check if a sub menu exist */
  if (item->sub_level == NULL)
  {
    /* link correctly the items together */
    item->prev_item->next_item = item->next_item;
    item->next_item->prev_item = item->prev_item;

    /* check and remove the parent dependance if exists */
    if (item->parent != NULL)
    {
      item->parent->sub_level = NULL;
    }
    free(item->name);
    //free(item->fct);
    free(item);
    
    return true;
  }
  /* when a submenu exists, remove it recursivly */
  else
  {
    Menu_Item_T *sub_level, *sub_next_item;

    sub_level = item->sub_level;
    sub_next_item = sub_level->next_item;
    
    /* loop on each item of the sub menu to delete it */
    while (sub_next_item != sub_level )
    {
      if (Del_Menu_Item(sub_next_item) == false)
      {
        return false;
      }
      sub_next_item = sub_level->next_item;
    }
  }
  return true;
} /* Del_Menu_Item */

/**
 * @brief  Check if the menu configuration is OK, if yes return the starting point of menu
 * @param  None
 * @retval bool State
 */
bool Def_Start_Menu_Item(Menu_Item_T * start_item)
{ 
  Menu_Item_T * next_item = start_item->next_item;
  int infinite_loop = 0;
  
  /* The starting stage of menu shouldn't have master above him */
  if (start_item->parent != NULL)
  {
    APP_ZB_DBG("The first stage of menu shouldn't have any master menu");
    return false;
  }

  /* The starting stage of menu shouldn't have master above him */
  while ((next_item != start_item ) && (infinite_loop++ <= 100))
  {
    if ( next_item->parent != NULL )
    {
      APP_ZB_DBG("The first stage of menu shouldn't have any master menu");
      return false;
    }
    next_item = next_item->next_item;
  }
  
  /* Infinite loop error */
  if (infinite_loop >= 100)
  {
    APP_ZB_DBG("Error : Last menu item isn't linked to first one");
    return false;
  }
  
  /* Check that all items are linked together */
  if (Check_Menu(start_item))
  {
    start_item->first_item_to_display = true;
    current_menu = start_item;
    Calc_Item_Nb();

    /* Launch the autorefresh for the UART */
    if (display_type & UART_DISPLAY)
    {
      // Timer to autorefresh menu on UART
  	  // Add to app_conf.h  (enum CFG_TimProcID_t)
      HW_TS_Create(CFG_TIM_MENU_REFRESH, &TS_ID_REFRESH_MENU_DISP, hw_ts_Repeated, Print_Menu);
      HW_TS_Start(TS_ID_REFRESH_MENU_DISP, HW_TS_MENU_REFRESH_DELAY);    /* Start auto display when config is ok*/
    }
    else
    {
      Print_Menu();
    }

    return true;
  }
  else
  {
    APP_ZB_DBG("Error at the check of menu");
    return false;
  }
} /* Def_Start_Menu_Item */


/* Private Functions Definition --------------------------------------------- */
// Internal checkers
/**
 * @brief  Check that every item is linked to another one
 * @param  menu_item to check
 * @retval bool State
 */
static bool Check_Menu( Menu_Item_T * menu_item) 
{
  const Menu_Item_T * first_item = menu_item;
  Menu_Item_T * temp_item = menu_item->next_item;
  int infinite_loop = 0;
  bool status_ok = true;
  bool first_item_already_exist = false;
  
  /* loop on each item of the menu to check it */
  while ((temp_item != first_item ) && (infinite_loop++ <= 100))
  {
    /* Check that sub menu items are linked together */
    if ( temp_item->sub_level != NULL ) 
    {
      /* Check for sub menu */
       status_ok = Check_Menu( temp_item->sub_level );
    }
    
    /* Check that there is no two items defined to be displayed first */
    if ( temp_item->first_item_to_display == true)
    {
      if (first_item_already_exist == true)
      {
        Clear_UART_Line();        /* Clear Terminal line*/
        APP_ZB_DBG("Error : Multiple item defined to be first to display");
        return 0;
      }
      else
      {
        first_item_already_exist = true;
      }
    }
    temp_item = temp_item->next_item;
  }
  
  /* Infinite loop error */
  if (infinite_loop >= 100)
  {
    APP_ZB_DBG("Error : Last menu item isn't linked to first one");
    status_ok = false;
  }
  
  return status_ok;
} /* Check_Menu */

/**
 * @brief  Compute the number of item in this menu stage
 * @param  None
 * @retval None
 */
static void Calc_Item_Nb(void)
{
  const Menu_Item_T * first_item = current_menu;
  Menu_Item_T * temp_item = current_menu->next_item;
  int infinite_loop = 0;

  nb_of_menu_item = 0;
  
  while ((temp_item != first_item ) && (infinite_loop++ <= 100))
  {
    nb_of_menu_item++;
    temp_item = temp_item->next_item;
  }
  nb_of_menu_item++;
  if(infinite_loop >= 100)
  {
    /* Should not happend, already checked in Check_Menu function */
    Clear_UART_Line();        /* Clear Terminal line*/ 
    APP_ZB_DBG("Erreur : Linked list not linked");
  }
} /* Calc_Item_Nb */

/**
 * @brief  Print the Menu. The Tera Term New Line setting should be set to LF
 * @param  None
 * @retval None
 */
static void Print_Menu(void)
{
  if (display_type & UART_DISPLAY)
  {
    Menu_Item_T * temp_item = current_menu;

    printf("\x1b[s");                                           //Save current position of cursor
    printf("\x1b[H\x1b[2K\x1b[m");                              //Set the terminal position on the top left of the terminal
                                                                //Delete line
                                                                //Reset all previous text config
    printf("\x1b[38;5;178m MENU :\x1b[m ");                     //Set Background and forground color for " MENU :" print
    
    /* Loop to display the first menu item */
    while (temp_item->first_item_to_display == false)
    {
      temp_item = temp_item->next_item;
    }
    
    /* Loop on each menu item and display it following order */
    for ( int i = 0; i < nb_of_menu_item; i++)
    {
      if (temp_item == current_menu)
      {
        /* Change the display following to if submenu or action to do */
        if (temp_item->fct == NULL)
        {
          printf(" \x1b[93m[%s]\x1b[m ",temp_item->name);
        }
        else
        {
          printf("  \x1b[93m%s\x1b[m  ",temp_item->name);          
        }
      }
      else
      {
        printf("  %s  ",temp_item->name);
      }       
      temp_item = temp_item->next_item;
    }
    /* return to the start of line */
    printf("\n\x1b[2K\x1b[u");                                  //Add one blank line, delete the text on it and retrieve last cursor position saved
  }
 
  /* DK LCD, display only the current menu */
  if (display_type & DK_LCD_DISPLAY)
  {
#ifdef LCD_H
    char Display_text[32];
	if (current_menu->fct == NULL)
	{
		sprintf(Display_text,"{%s}", current_menu->name);
	}
	else
	{
	    sprintf(Display_text,"[%s]", current_menu->name);         
	}	
    UTIL_LCD_ClearStringLine(DK_LCD_MENU_LINE);
    UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_MENU_LINE), (uint8_t *) Display_text, CENTER_MODE);
    BSP_LCD_Refresh(0);
#else
    APP_ZB_DBG("No LCD config");
#endif    
  }
} /* Print_Menu */

/**
 * @brief  Clear the menu in UART. The Tera Term New Line setting should be set to LF
 * @param  None
 * @retval None
 */
void Clear_UART_Line(void)
{
  if (display_type & UART_DISPLAY)
  {
  	printf("\x1b[2K");    /* Clear Terminal line*/
  }
} /* Clear_UART_LINE */

