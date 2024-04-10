/**
  ************************************************************************************************
  * @file    readme.txt 
  * @author  MCD Application Team
  * @brief   Arduino Motor Shield Rev3 driver
  *****************

Purpose :
========
This driver has been created to control the channel A of the Arduino Motor Shield (AMS).
It configures 2 GPIOs (Direction and brake pin), the PWM to manage the speed of the motor and the ADC to stop it if it is blocked.

Project configuration :
======================
Add the include path of AMS.c, AMS_ADC.c and AMS_PWM.c in your project (like the IAR file <project-name>.ewp). You shouldn't have to modify those files.
Then add the file AMS_conf.h in you project file directory, this is where you will config your AMS.
If not define, you have to manually uncomment HAL_ADC_MODULE_ENABLED and HAL_TIM_MODULE_ENABLED in stm32wbxx_hal_conf.h
If files stm32wbxx_hal_adc.c, stm32wbxx_hal_adc_ex.c, stm32wbxx_hal_tim.c aren't included as part of the project, add it too.
In you application, include AMS.h. This is the only file you need to include.

Global configuration :
=====================
The file AMS_conf.h is used to config overall Arduino motor shield driver.
You can:
	- Select pre config GPIO/Timer/ADC for Nucleo or DK boards
	- disable ADC
	- change pre config rotating direction of motor.
	- Change TIM1 Channel
		--> Channel depends of corresponding AMS PWM GPIO. For example, if you want channel 5, you have to modifie :
		-->	#define AMS_TIM_CHANNELx                TIM_CHANNEL_5
	- Change ADC1 Channel
		--> Channel depends of corresponding AMS ADC GPIO. For example, if you want channel 5, you have to modifie :
		-->	#define AMS_ADC_CHANNEL                 ADC_CHANNEL_5
	- Change DMA1 Channel
		--> In basic example, DMA1 channel 1 and 2 are used for lpuart and usart, if you want channel 5, you have to modifie
		-->	#define AMS_DMA1_CHANNELx               DMA1_Channel5
			#define AMS_DMA1_Channelx_IRQn          DMA1_Channel5_IRQn
			#define AMS_DMA1_CHANNELx_IRQHandler    DMA1_Channel5_IRQHandler

ADC configuration :
==================
As default, ADC is used to control that the current consumption doesn't exceed a value. It is automatically set up with an ADC watchdog. 
You must configure the max current window and if exceeded, the IT will be raised. 
So you have to rewrite the function "void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)" if you want to take into account the IT
	--> in the Arduino motor shield Rev-3, this is supposed to work as follow, max current per Channel (A and B on motor shield) is 2A.
	    It corresponds to 3,3V and with our 12 Bits ADC it corresponds to 0xFFF (4095)
	--> If you want that your motor doesn't exceed 0.2A, you have to configure the HighTreshold to 0x199 (dec : 409 ==> 0.33V ==> 0.2A)
		--> A problem appear at the start of the motor, a current draw occurs when the motor start that trigger the ADC watchdog if the threshold isn't high enough.
			--> to solve that issue: get the HAL_Tick (and store it in tick_motor_start for example) at the start of the motor, and treat the IT only if :
				( (HAL_GetTick() - tick_motor_start) / (HW_TS_SERVER_1ms_NB_TICKS * 200) > my_time_in_ms)
			--> 200 ms should be enough (It could even be lowered, not tested)
		--> The value of max_Treshold isn't working as it is supposed to do, you should try some value on your own.
		--> Otherwise, if you have to lift something, you should use ams_adc_change_treshold_value before every ams_start_motor_up/down call because motor_up will demands more current than motor_down.

Default configuration for DK/Nucleo boards:
==========================================
Reminder Channel A config :
	--> for STM32WB55RG-Nucleo
		--> DIR_A   on D12 --> PA6
		--> BREAK_A on D9  --> PA9
		--> PWM_A   on D3  --> PA10 --> TIM1 Channel_3
		--> ADC_A   on A0  --> PC0
	--> for STM32WB5MM-DK
		--> DIR_A   on D12 --> PB4
		--> BREAK_A on D9  --> PD15
		--> PWM_A   on D3  --> PD14 --> TIM1 Channel_1
		--> ADC_A   on A0  --> PC3

In case you are not using the channel A of the Motor_shield, you have to repeat what has been done in the code for the channel B
Channel B config : 
	--> for STM32WB55RG-Nucleo
		--> DIR_B   on D13 --> PA5
		--> BREAK_B on D8  --> PC12
		--> PWM_B   on D11 --> PA7 --> TIM1 Channel_1N or TIM17 Channel1
		--> ADC_B   on A1  --> PC1
	--> for STM32WB5MM-DK
		--> DIR_B   on D13 --> PA1
		--> BREAK_B on D8  --> PD13
		--> PWM_B   on D11 --> PA7 --> TIM1 Channel_1N or TIM17 Channel1
		--> ADC_B   on A1  --> PA2

Arduino Motor Shield Doc :
==========================
Arduino Motor Shield Doc : https://docs.arduino.cc/tutorials/motor-shield-rev3/msr3-controlling-dc-motor

Example :
=========
Two examples are given, one for the Nucleo and the other for the DK.
