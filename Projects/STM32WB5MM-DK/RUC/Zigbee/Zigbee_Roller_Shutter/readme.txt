/**
  @page Zigbee_Roller_Shutter application
  
  @verbatim
  ******************************************************************************
  * @file    Zigbee/Zigbee_Roller_Shutter/readme.txt 
  * @author  MCD Application Team
  * @brief   Description of the Zigbee RUC Window Covering Example. 
  ******************************************************************************
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @endverbatim

@par Application Description 

This application act as a Roller Shutter.
Note this application was developped as part of the Zigbee RUC Window Covering example 
to create a basic Zigbee network and not for an industrial application.
This device is a ZR with two Endpoints:
	- A Roller Shutter with the following Zigbee clusters: Window covering, occupancy.
	- A Light Bulb with the the following Zigbee clusters: OnOff, level

@par Keywords

Zigbee, RUC, Realistic Use Cases, Light, OnOff, Roller Shutter, Window Covering
 
@par Hardware and Software environment

  - This application runs on STM32WB5MM Discovery Kit devices
  - This application was developped with CubeWB package v1.18.0 and IAR 9.20.4.
  - STM32WB5MM Discovery Kit board Set-up    
       - Connect the Board to your PC with a USB cable type A to mini-B to ST-LINK connector (USB_STLINK).

    
@par How to use it ? 


This application requires having the stm32wb5x_Zigbee_FFD_fw.bin binary flashed on the Wireles Coprocessor.
If it is not the case, you need to use STM32CubeProgrammer to load the appropriate binary.
For the detailed procedure to change the Wireless Coprocessor binary,
see following wiki for Hardware setup:
Refer to https://wiki.st.com/stm32mcu/wiki/Connectivity:STM32WB_Build_Zigbee_Project

In order to make the program work, you must do the following :
  - Open your toolchain
  - Rebuild all files and flash the board with executable file

1. Flash the Zigbee wireles stack **Full** on the STM32WB5MM-DK board.

2. Build the application and flash the board with it.

3. You can now join a Zigbee network and bind with Roller Shutter remote and use it.

Available Wiki pages 
  - https://wiki.st.com/stm32mcu/wiki/Connectivity:Zigbee_Realistic_Use_Case_Window_Covering

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */