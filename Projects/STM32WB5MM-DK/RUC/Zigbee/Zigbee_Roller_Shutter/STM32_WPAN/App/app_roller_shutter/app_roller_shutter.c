/**
  ******************************************************************************
  * @file    app_roller_shutter.c
  * @author  Zigbee Application Team
  * @brief   Application interface for Roller shutter Endpoint
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
#include "stm32_seq.h"

/* board dependancies */
#include "stm32wb5mm_dk_lcd.h"
#include "stm32_lcd.h"

/* HW dependancies*/
#include "AMS.h"

/* service dependancies */
#include "app_roller_shutter_cfg.h"
#include "app_core.h"
#include "app_zigbee.h"

/* Private defines -----------------------------------------------------------*/
#define IDENTIFY_MODE_DELAY              30U
#define HW_TS_IDENTIFY_MODE_DELAY         (IDENTIFY_MODE_DELAY * HW_TS_SERVER_1S_NB_TICKS)

/* External variables --------------------------------------------------------*/
extern App_Zb_Info_T app_zb_info;
extern bool id_mode_on;
extern ADC_HandleTypeDef hadc1;

/* Private Variable ----------------------------------------------------------*/
static uint8_t TS_ID_STOP_MOTOR;
static uint8_t TS_ID_STOP_MOTOR_BOT_END_SENSOR;
static volatile uint32_t tick_start;

/* Application Variable-------------------------------------------------------*/
Roller_Shutter_Control_T app_Roller_Shutter_Control =
{
  .state = IDLE,
  .secure_timer_up   = DEFAULT_secure_timer_up,
  .secure_timer_down = DEFAULT_secure_timer_down,
  .PWM_Motor_Speed   = DEFAULT_PWM_Motor_Speed,

  .ADC_TresholdHigh_Up   = DEFAULT_ADC_TresholdHigh_Up,
  .ADC_TresholdHigh_Down = DEFAULT_ADC_TresholdHigh_Down,
  .ADC_TresholdLow       = DEFAULT_ADC_TresholdLow,
};

/* Application function ------------------------------------------------------*/
/* Set/Get */
void App_Roller_Shutter_Set_State                          (Shutter_State_T state);
Shutter_State_T App_Roller_Shutter_Get_State                (void);

/* Finding&Binding function Protoype */
static void App_Roller_Shutter_Identify_cb(struct ZbZclClusterT *cluster, enum ZbZclIdentifyServerStateT state, void *arg);
static void App_Roller_Shutter_FindBind_cb(enum ZbStatusCodeT status, void *arg);

/* Motor Control */
static void App_Roller_Shutter_Motor_Limit_Switch_GPIO_Init(void);
static void App_Roller_Shutter_Motor_Control_Task          (void);
// static void alert_too_high_current                 (void);

/* Occupancy detection for Window covering action */
static void App_Roller_Shutter_Occupancy_Task(void);

/* Set/Get Applications ----------------------------------------------------- */
/**
 * @brief Set window State for the Motor control
 * 
 * @param state of the roller shutter
 */
void App_Roller_Shutter_Set_State(Shutter_State_T state)
{
  app_Roller_Shutter_Control.state = state;
} /* App_Roller_Shutter_Set_State */

/**
 * @brief Get the Window for the Motor control
 * 
 * @return Shutter_State_T 
 */
Shutter_State_T App_Roller_Shutter_Get_State (void)
{
  return app_Roller_Shutter_Control.state;
} /* App_Roller_Shutter_Get_State */


/* EndPoint Configuration ----------------------------------------------------*/
/**
 * @brief  Configure and register Zigbee application endpoints, window covering callbacks
 * @param  zb Zigbee stack instance
 * @retval None
 */
void App_Roller_Shutter_Cfg_Endpoint(struct ZigBeeT *zb)
{
  struct ZbApsmeAddEndpointReqT      req;
  struct ZbApsmeAddEndpointConfT     conf;
  
  /* Endpoint: ROLLER_SHUTTER_ENDPOINT */
  memset(&req, 0, sizeof(req));
  req.profileId = ZCL_PROFILE_HOME_AUTOMATION;
  req.deviceId  = ZCL_DEVICE_WINDOW_COVERING_DEVICE;
  req.endpoint  = ROLLER_SHUTTER_ENDPOINT;
  ZbZclAddEndpoint(zb, &req, &conf);
  assert(conf.status == ZB_STATUS_SUCCESS);

  /* Idenfity server */
  app_Roller_Shutter_Control.identify_server = ZbZclIdentifyServerAlloc(zb, ROLLER_SHUTTER_ENDPOINT, NULL);
  assert(app_Roller_Shutter_Control.identify_server != NULL);
  assert(ZbZclClusterEndpointRegister(app_Roller_Shutter_Control.identify_server)!= NULL);
  ZbZclIdentifyServerSetCallback(app_Roller_Shutter_Control.identify_server, App_Roller_Shutter_Identify_cb); 

  /* Window Covering Server */
  app_Roller_Shutter_Control.app_Window_Covering_Control = App_Roller_Shutter_Window_Covering_Config(zb);

  /*  Occupancy Client */
  app_Roller_Shutter_Control.app_Occupancy = App_Roller_Shutter_Occupancy_Cfg(zb);

  /* make a local pointer of Zigbee stack to read easier */
  app_Roller_Shutter_Control.zb = zb;

  /* Motor Control Initialization */
  if (ams_init( app_Roller_Shutter_Control.PWM_Motor_Speed,app_Roller_Shutter_Control.ADC_TresholdHigh_Up) == 0)
  {
    APP_ZB_DBG("Error : AMS driver init failed");
  }  
  App_Roller_Shutter_Motor_Limit_Switch_GPIO_Init();  
  
  /* Task/Timer for Motor Control Init */
  UTIL_SEQ_RegTask(1U << CFG_TASK_MOTOR_CONTROL, UTIL_SEQ_RFU, App_Roller_Shutter_Motor_Control_Task);
  UTIL_SEQ_RegTask(1U << CFG_TASK_LIMIT_SWITCH,  UTIL_SEQ_RFU, App_Roller_Shutter_Stop);
  HW_TS_Create(CFG_TIM_PROC_ID_ISR, &TS_ID_STOP_MOTOR, hw_ts_SingleShot, App_Roller_Shutter_Stop);
  HW_TS_Create(CFG_TIM_PROC_ID_ISR, &TS_ID_STOP_MOTOR_BOT_END_SENSOR, hw_ts_SingleShot, App_Roller_Shutter_Stop);
  //HW_TS_Create(CFG_TIM_PROC_ID_ISR, &TS_ID_ADC_STOP, hw_ts_Repeated, alert_too_high_current);
  UTIL_SEQ_RegTask(1U << CFG_TASK_ROLLER_SHUTTER_OCCUPANCY_EVT,  UTIL_SEQ_RFU, App_Roller_Shutter_Occupancy_Task);

  UTIL_LCD_ClearStringLine(DK_LCD_SHUTTER_DISP);
  UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_SHUTTER_DISP), (uint8_t *) "SHUTTER : STOP", CENTER_MODE);
  BSP_LCD_Refresh(0);
} /* App_Roller_Shutter_Cfg_Endpoint */

/**
 * @brief  Restore the state at startup from persistence
 * @param  None
 * @retval stack status code
 */
enum ZclStatusCodeT App_Roller_Shutter_Restore_State(void)
{
  if (App_Roller_Shutter_Window_Covering_Restore_State() != ZCL_STATUS_SUCCESS)
    return ZCL_STATUS_FAILURE;
  
  if (HAL_GPIO_ReadPin(LIMIT_SWITCH_TOP_GPIO_Port, LIMIT_SWITCH_TOP_PIN) == 0)
  {
    App_Roller_Shutter_Set_State(TOP_REACHED);
    APP_ZB_DBG("Detect Top limit switch");
  }
  else if (HAL_GPIO_ReadPin(LIMIT_SWITCH_BOT_GPIO_Port, LIMIT_SWITCH_BOT_PIN) == 0)
  {
    App_Roller_Shutter_Set_State(BOTTOM_REACHED);
    APP_ZB_DBG("Detect Bottom limit switch");
  }
  else
  {
    App_Roller_Shutter_Set_State(IDLE);
    APP_ZB_DBG("Roller Shutter position between Top-Down, no limit switch detected");
  }
  APP_ZB_DBG("Read back cluster information : SUCCESS");  

  return ZCL_STATUS_SUCCESS;
} /* app_window_covering_Restore_State */


/* Identify & FindBind actions -----------------------------------------------*/
/**
 * @brief Put Identify cluster into identify mode to response at the binding queries emitted
 * 
 */
void App_Roller_Shutter_IdentifyMode(void)
{  
  if (id_mode_on == true)
  {
    APP_ZB_DBG("WARNING: AN ID MODE IS ALREADY LAUNCHED on another EndPoint");
    return;
  }
  
  if (app_zb_info.join_status != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Error, device not on a network");
    return;
  }

  ZbZclIdentifyServerSetTime(app_Roller_Shutter_Control.identify_server, IDENTIFY_MODE_DELAY);
} /* App_Roller_Shutter_Covering_IdentifyMode */

/**
 * @brief Callback to manage easely the identify mode and the display
 * Needed to avoid all endpoints are at the same time in identify mode
 * 
 * @param cluster Identify pointer
 * @param state of the Identify mode
 * @param arg NULL in all cases
 */
static void App_Roller_Shutter_Identify_cb(struct ZbZclClusterT *cluster, enum ZbZclIdentifyServerStateT state, void *arg)
{
  char disp_identify[18];
  switch (state)
  {
    case ZCL_IDENTIFY_START :
      id_mode_on = true;
      APP_ZB_DBG("Turn on Identify Mode for %ds", IDENTIFY_MODE_DELAY);
      /* Display info on LCD */
      UTIL_LCD_ClearStringLine(DK_LCD_STATUS_LINE);
      sprintf(disp_identify, "Identify Mode %2ds", IDENTIFY_MODE_DELAY);
      UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_STATUS_LINE), (uint8_t *)disp_identify, CENTER_MODE);
      BSP_LCD_Refresh(0);
      break;

    case ZCL_IDENTIFY_STOP:
      id_mode_on = false;
      APP_ZB_DBG("Turn off Identify Mode");
      UTIL_SEQ_SetTask(1U << CFG_TASK_LCD_CLEAN_STATUS, CFG_SCH_PRIO_1);
      break;

    default :
      APP_ZB_DBG("Identification mode CallBack state unknown, please investigate");
      break;
  }
} /* App_Roller_Shutter_Identify_cb */

/**
 * @brief  Start Finding and Binding process as an initiator for Occupancy cluster
 * Call App_Roller_Shutter_FindBind_cb when successfull to configure correctly the binding
 * 
 */
void App_Roller_Shutter_FindBind(void)
{
  uint8_t status = ZCL_STATUS_FAILURE;

  /* Check if the device is on a Zigbee network or not */
  if (app_zb_info.join_status != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Error, device not on a network");
    return;
  }

  APP_ZB_DBG("Initiate F&B");
  
  status = ZbStartupFindBindStart(app_Roller_Shutter_Control.zb, &App_Roller_Shutter_FindBind_cb, NULL);

  if (status != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG(" Error, cannot start Finding & Binding, status = 0x%02x", status);
  }
} /* App_Roller_Shutter_FindBind */

/**
 * @brief  Task called after F&B process to configure a report and update the local status.
 *         Will call the configuration report for the good clusterId.
 * @param  None
 * @retval None
 */
static void App_Roller_Shutter_FindBind_cb(enum ZbStatusCodeT status, void *arg)
{
  // Retry if not succes ????
  if (status != ZB_STATUS_SUCCESS)
  {
    APP_ZB_DBG("Error while F&B | error code : 0x%02X", status);
    return;
  }
  else
  {
    struct ZbApsmeBindT entry;
  
    APP_ZB_DBG(" Item |   ClusterId | Long Address     | End Point");
    APP_ZB_DBG(" -----|-------------|------------------|----------");

    /* Loop only on the new binding element */
    for (uint8_t i = 0;; i++)
    {
      if (ZbApsGetIndex(app_Roller_Shutter_Control.zb, ZB_APS_IB_ID_BINDING_TABLE, &entry, sizeof(entry), i) != ZB_APS_STATUS_SUCCESS)
      {
        break;
      }
      if (entry.srcExtAddr == 0ULL)
      {
        continue;
      }
    
      /* display binding infos */
      APP_ZB_DBG("  %2d  |     0x%03x   | %016llx |   %2d", i, entry.clusterId, entry.dst.extAddr, entry.dst.endpoint);
        
      // Report on the Cluster selected to know when a modification status
      switch (entry.clusterId)
      {
        case ZCL_CLUSTER_MEAS_OCCUPANCY:
          // start report config proc
          App_Roller_Shutter_Occupancy_ReportConfig( &entry.dst);
          break;
        default:
          break;
      }
    }  
  }
} /* App_Roller_Shutter_FindBind_cb */

/**
 * @brief  For debug purpose, display the local binding table information
 * 
 */
void App_Roller_Shutter_Bind_Disp (void)
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
    if (ZbApsGetIndex(app_Roller_Shutter_Control.zb , ZB_APS_IB_ID_BINDING_TABLE, &entry, sizeof(entry), i) != ZB_APS_STATUS_SUCCESS)
    {
      break;
    }
    if (entry.srcExtAddr == 0ULL)
    {
      continue;
    }
    if (entry.clusterId != ZCL_CLUSTER_WINDOW_COVERING)
    {
      continue;
    }    
    APP_ZB_DBG(" %2d  | %016llx |   0x%04x  |    0x%04x    |    0x%04x", i, entry.dst.extAddr, entry.clusterId, entry.srcEndpt, entry.dst.endpoint);
  }
  APP_ZB_DBG("-----------------------------------------------------------------\n\r");
} /* App_Roller_Shutter_Bind_Disp */


/* Window Motor Applications ------------------------------------------------ */
/**
  * @brief  GPIO Init for Limit Switch 
  * @param  None
  * @retval None
  */
static void App_Roller_Shutter_Motor_Limit_Switch_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  APP_ZB_DBG("Limit Switch IT Register");
  
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  
  /*Configure GPIO pins : PE4 --> Arduino D4 = Limit Switch */
  GPIO_InitStruct.Pin  = LIMIT_SWITCH_TOP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(LIMIT_SWITCH_TOP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC5 --> Arduino A5 = Limit Switch */
  GPIO_InitStruct.Pin  = LIMIT_SWITCH_BOT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(LIMIT_SWITCH_BOT_GPIO_Port, &GPIO_InitStruct);
  
  /* Enable and set  EXTI Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(LIMIT_SWITCH_TOP_EXTIx_IRQn, 0x0FUL, 0x0F);
  HAL_NVIC_EnableIRQ  (LIMIT_SWITCH_TOP_EXTIx_IRQn);
  
  HAL_NVIC_SetPriority(LIMIT_SWITCH_BOT_EXTIx_IRQn, 0x0FUL, 0x0E);
  HAL_NVIC_EnableIRQ  (LIMIT_SWITCH_BOT_EXTIx_IRQn); 
} /* App_Roller_Shutter_Motor_Limit_Switch_GPIO_Init */

/**
 * @brief  This function handles External line
 *         interrupt request of End Sensor at the Top limit switch
 * @param  None
 * @retval None
 */
void LIMIT_SWITCH_TOP_EXTIx_IRQHandler(void)
{
  if ((App_Roller_Shutter_Window_Covering_Get_Cmd() == ZCL_WNCV_COMMAND_UP) &&
      (App_Roller_Shutter_Get_State() == RUN))
  {
    APP_ZB_DBG("TOP IT");
    /* Stop Watchdog Timer */
    HW_TS_Stop(TS_ID_STOP_MOTOR);
    App_Roller_Shutter_Set_State(TOP_REACHED);
    UTIL_SEQ_SetTask(1U << CFG_TASK_LIMIT_SWITCH, CFG_SCH_PRIO_1);
  }
  __HAL_GPIO_EXTI_CLEAR_IT(LIMIT_SWITCH_TOP_PIN);
} /* LIMIT_SWITCH_TOP_EXTIx_IRQHandler */

/**
 * @brief  This function handles External line
 *         interrupt request of End Sensor at the bottom limit switch
 * @param  None
 * @retval None
 */
void LIMIT_SWITCH_BOT_EXTIx_IRQHandler(void)
{
  if ((App_Roller_Shutter_Window_Covering_Get_Cmd() == ZCL_WNCV_COMMAND_DOWN) &&
      (App_Roller_Shutter_Get_State() == RUN))
  {
    APP_ZB_DBG("BOTTOM IT");
    /* Stop Watchdog Timer */
    HW_TS_Stop(TS_ID_STOP_MOTOR);
    App_Roller_Shutter_Set_State(BOTTOM_REACHED);
    HW_TS_Start(TS_ID_STOP_MOTOR_BOT_END_SENSOR, HW_TS_BOTTOM_END_DELAY);     
  }
  __HAL_GPIO_EXTI_CLEAR_IT(LIMIT_SWITCH_BOT_PIN);
} /* LIMIT_SWITCH_BOT_EXTIx_IRQHandler */


#if 0
/**
  * @brief  Function to make led blinkt after ADC IT raised
  * 
  * @param  None
  * @retval None
  */
static void alert_too_high_current(void)
{
  static uint8_t led_state = 0;

  if (led_state++ < 6)
  {
    if (led_state == 1)
    {
      APP_ZB_DBG("To much current needed | Secure motor entered"); 
      if ( ZbZclAttrIntegerWrite(app_Roller_Shutter_Control.window_server, ZCL_WNCV_SVR_ATTR_CURR_POS_LIFT_PERCENT, ZCL_COMMAND_ADC_STOP) != ZCL_STATUS_SUCCESS)
      { 
        APP_ZB_DBG("Error ZbZclAttrIntegerWrite");  
      }      
    }
  }
  else
  {
    /* Motor Control */
    led_state = 0;
    HW_TS_Stop(TS_ID_ADC_STOP);
  }
} static /* alert_too_high_current */
#endif

#if 0
/**
  * @brief  Analog watchdog callback in non blocking mode.
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{
  /* Avoid callback at the start of the motor */
  if (  (HAL_GetTick() - tick_start) / HW_TS_SERVER_1ms_NB_TICKS > 500)
  {
    /* Stop PWM */
    if (ams_stop_motor() == false)
       APP_ZB_DBG("Error while trying to stop motor");
  
    /* Call Task to update app state */
    UTIL_SEQ_SetTask(1U << CFG_TASK_LIMIT_SWITCH, CFG_SCH_PRIO_1);  // Start App_Roller_Shutter_Stop
    HW_TS_Start(TS_ID_ADC_STOP, 500 * HW_TS_SERVER_1ms_NB_TICKS);   // Led Toggle + Dbg msg
    
    tick_start = HAL_GetTick(); /* Reset tick start to avoid too many call */
  }
}
#endif

/**
  * @brief  Global FSM to control Motor associated to the Window Covering Device
  * 
  */
static void App_Roller_Shutter_Motor_Control_Task(void)
{
  switch ( App_Roller_Shutter_Window_Covering_Get_Cmd() )
  {
    case ZCL_WNCV_COMMAND_UP :
      if (App_Roller_Shutter_Get_State() == TOP_REACHED)
      {
        APP_ZB_DBG("The window is already at the top");
        return;
      }
      else
      {
        App_Roller_Shutter_Set_State(RUN);
      }

      /* Init anti-pitch detection */
      ams_adc_change_treshold_value(app_Roller_Shutter_Control.ADC_TresholdHigh_Up, app_Roller_Shutter_Control.ADC_TresholdLow);

      if (ams_start_motor_up()) {APP_ZB_DBG("Moves Window Up"); }
      else
        APP_ZB_DBG("Error in Cmd up");
      
      /* Launches Watchdog Timer to stop after too long time */
      HW_TS_Start(TS_ID_STOP_MOTOR, app_Roller_Shutter_Control.secure_timer_up * HW_TS_SERVER_1ms_NB_TICKS);

      /* Display current action on LCD */
      UTIL_LCD_ClearStringLine(DK_LCD_SHUTTER_DISP);
      UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_SHUTTER_DISP), (uint8_t *) "SHUTTER : UP", CENTER_MODE);
      BSP_LCD_Refresh(0);
      break;
      
    case ZCL_WNCV_COMMAND_DOWN :
      if (App_Roller_Shutter_Get_State() == BOTTOM_REACHED)
      {
        APP_ZB_DBG("The window is already at the bottom");
        return;
      }
      else
      {
        App_Roller_Shutter_Set_State(RUN);
      }

      /* Init anti-pitch detection */
      ams_adc_change_treshold_value(app_Roller_Shutter_Control.ADC_TresholdHigh_Down, app_Roller_Shutter_Control.ADC_TresholdLow);
      
      if (ams_start_motor_down()) {APP_ZB_DBG("Moves Window down"); }
      else
        APP_ZB_DBG("Error in Cmd down");

      /* Launches Watchdog Timer to stop after too long time */
      HW_TS_Start(TS_ID_STOP_MOTOR, app_Roller_Shutter_Control.secure_timer_down * HW_TS_SERVER_1ms_NB_TICKS);

      /* Display current action on LCD */
      UTIL_LCD_ClearStringLine(DK_LCD_SHUTTER_DISP);
      UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_SHUTTER_DISP), (uint8_t *) "SHUTTER : DOWN", CENTER_MODE);
      BSP_LCD_Refresh(0);    
      break;
      
    case ZCL_WNCV_COMMAND_STOP :
      if (App_Roller_Shutter_Get_State() == RUN)
      {
        App_Roller_Shutter_Set_State(IDLE);
      }
      // else
      // {
      //   APP_ZB_DBG("The window is not mooving");
      // }

      if (ams_stop_motor()) {APP_ZB_DBG("Stop Window moving");}
      else
        APP_ZB_DBG("Error in Cmd Stop");

      /* Display current action on LCD */
      UTIL_LCD_ClearStringLine(DK_LCD_SHUTTER_DISP);
      UTIL_LCD_DisplayStringAt(0, LINE(DK_LCD_SHUTTER_DISP), (uint8_t *) "SHUTTER : STOP", CENTER_MODE);
      BSP_LCD_Refresh(0);    
      break;
  }

  tick_start = HAL_GetTick();
} /* App_Roller_Shutter_Motor_Control_Task */

/**
 * @brief action done by event of the Occupancy sensor
 *  should be depend of the application target
 * 
 */
static void App_Roller_Shutter_Occupancy_Task(void)
{
  /* Update the Window covering server according to the Sensor information */
  if (app_Roller_Shutter_Control.app_Occupancy->Occupancy)
  {
    App_Roller_Shutter_Up();
  }
  else
  {
    App_Roller_Shutter_Down();
  }
}

/* Window Menu Applications ------------------------------------------------ */
/**
  * @brief  Function to stop motor when reach the sensor limit
  * Calling by Task/Timer
  * 
  */
void App_Roller_Shutter_Stop(void)
{
  enum ZclStatusCodeT status = ZCL_STATUS_FAILURE;

  /* Call directly the cluster implementation */
  status = Window_Server_Stop_Cb(app_Roller_Shutter_Control.app_Window_Covering_Control->window_server, NULL, NULL, NULL);
  if(status != ZCL_STATUS_SUCCESS)
    APP_ZB_DBG("Error during execution of cluster Stop command");

} /* App_Roller_Shutter_Stop */

/**
  * @brief  Function to stop motor when reach the sensor limit
  * Calling by Task/Timer
  * 
  */
void App_Roller_Shutter_Up(void)
{
  enum ZclStatusCodeT status = ZCL_STATUS_FAILURE;

  /* Call directly the cluster implementation */
  status = Window_Server_Up_Cb(app_Roller_Shutter_Control.app_Window_Covering_Control->window_server, NULL, NULL, NULL);
  if(status != ZCL_STATUS_SUCCESS)
    APP_ZB_DBG("Error during execution of cluster Stop command");
} /* App_Roller_Shutter_Up */

/**
  * @brief  Function to stop motor when reach the sensor limit
  * Calling by Task/Timer
  * 
  */
void App_Roller_Shutter_Down(void)
{
  enum ZclStatusCodeT status = ZCL_STATUS_FAILURE;

  /* Call directly the cluster implementation */
  status = Window_Server_Down_Cb(app_Roller_Shutter_Control.app_Window_Covering_Control->window_server, NULL, NULL, NULL);
  if(status != ZCL_STATUS_SUCCESS)
    APP_ZB_DBG("Error during execution of cluster Stop command");
} /* App_Roller_Shutter_Down */

/* Typically Methods/Functions to not write outside the Endpoint perimeter */
/**
 * @brief Function to increase the Watchdog Timer during raising of the roller shutter
 * 
 */
void App_Roller_Shutter_timer_motorup_up(void)
{
  app_Roller_Shutter_Control.secure_timer_up += SECURE_TIMER_STEP; 
  APP_ZB_DBG("New secure time : %d",app_Roller_Shutter_Control.secure_timer_up);
} /* App_Roller_Shutter_timer_motorup_up */

/**
 * @brief Function to decrease the Watchdog Timer during raising of the roller shutter
 * 
 */
void App_Roller_Shutter_timer_motorup_down(void)
{ 
  app_Roller_Shutter_Control.secure_timer_up -= SECURE_TIMER_STEP; 
  APP_ZB_DBG("New secure time : %d",app_Roller_Shutter_Control.secure_timer_up);  
} /* app_timer_motorup_down */

/**
 * @brief Function to increase the Watchdog Timer during lowering of the roller shutter
 * 
 */
void App_Roller_Shutter_timer_motordown_up(void)
{
  app_Roller_Shutter_Control.secure_timer_down += SECURE_TIMER_STEP; 
  APP_ZB_DBG("New secure time : %d",app_Roller_Shutter_Control.secure_timer_down);    
} /* app_timer_motordown_up */

/**
 * @brief Function to decrease the Watchdog Timer during lowering of the roller shutter
 * 
 */
void App_Roller_Shutter_timer_motordown_down(void)
{
  app_Roller_Shutter_Control.secure_timer_down -= SECURE_TIMER_STEP; 
  APP_ZB_DBG("New secure time : %d",app_Roller_Shutter_Control.secure_timer_down); 
} /* app_timer_motordown_down */

/**
 * @brief Function to increase the motor speed
 * 
 */
void App_Roller_Shutter_motor_speed_up(void)
{
  app_Roller_Shutter_Control.PWM_Motor_Speed += PWM_MOTOR_SPEED_STEP;
  ams_pwm_change_duty_cycle(app_Roller_Shutter_Control.PWM_Motor_Speed);
  APP_ZB_DBG("New PWM : %d",app_Roller_Shutter_Control.PWM_Motor_Speed);  
} /* app_motor_speed_up */

/**
 * @brief Function to decrease the motor speed
 * 
 */
void App_Roller_Shutter_motor_speed_down(void)
{
  app_Roller_Shutter_Control.PWM_Motor_Speed -= PWM_MOTOR_SPEED_STEP;
  ams_pwm_change_duty_cycle(app_Roller_Shutter_Control.PWM_Motor_Speed);  
  APP_ZB_DBG("New PWM : %d",app_Roller_Shutter_Control.PWM_Motor_Speed);   
} /* app_motor_speed_down */

/**
 * @brief Function to increase the threshold current to automatically stop the motor when raising the roller shutter
 * 
 */
void App_Roller_Shutter_adc_treshold_up(void)
{
  app_Roller_Shutter_Control.ADC_TresholdHigh_Up += ADC_TRESHOLD_STEP;
  if (ams_adc_change_treshold_value(app_Roller_Shutter_Control.ADC_TresholdHigh_Up, app_Roller_Shutter_Control.ADC_TresholdLow) == false)
     APP_ZB_DBG("Erro while init ADC")
  APP_ZB_DBG("New ADC High Treshold value : reel : %d vs %d", LL_ADC_GetAnalogWDThresholds(hadc1.Instance, ADC_ANALOGWATCHDOG_1, LL_ADC_AWD_THRESHOLD_HIGH) ,app_Roller_Shutter_Control.ADC_TresholdHigh_Up );
} /* app_adc_treshold_up */

/**
 * @brief Function to decrease the threshold current to automatically stop the motor when raising the roller shutter
 * 
 */
void App_Roller_Shutter_adc_treshold_down(void)
{ 
  app_Roller_Shutter_Control.ADC_TresholdHigh_Up -= ADC_TRESHOLD_STEP;
  if (ams_adc_change_treshold_value(app_Roller_Shutter_Control.ADC_TresholdHigh_Up, app_Roller_Shutter_Control.ADC_TresholdLow) == false)
     APP_ZB_DBG("Erro while init ADC");
  APP_ZB_DBG("New ADC High Treshold value : reel : %d vs %d", LL_ADC_GetAnalogWDThresholds(hadc1.Instance, ADC_ANALOGWATCHDOG_1, LL_ADC_AWD_THRESHOLD_HIGH) ,app_Roller_Shutter_Control.ADC_TresholdHigh_Up );
} /* app_adc_treshold_down */

// TODO May be to use in futur developpment
// void     App_Roller_Shutter_Secure_TimerUp_Set  (uint16_t secure_timer_up)
// {
//   app_Roller_Shutter_Control.secure_timer_up = secure_timer_up;
// }
// uint16_t App_Roller_Shutter_Secure_TimerUp_Get  (void)
// {
//   return app_Roller_Shutter_Control.secure_timer_up;
// }
// void     App_Roller_Shutter_Secure_TimerDown_Set(uint16_t secure_timer_down)
// {
//   app_Roller_Shutter_Control.secure_timer_down = secure_timer_down;
// }
// uint16_t App_Roller_Shutter_Secure_TimerDown_Get(void)
// {
//   return app_Roller_Shutter_Control.secure_timer_down;
// }
// void     App_Roller_Shutter_PWM_Motor_Speed_Set (uint32_t PWM_Motor_Speed)
// {
//   app_Roller_Shutter_Control.PWM_Motor_Speed = PWM_Motor_Speed;
// }
// uint32_t App_Roller_Shutter_PWM_Motor_Speed_Get (void)
// {
//   return app_Roller_Shutter_Control.PWM_Motor_Speed;
// }

