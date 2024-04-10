# **STM32WB Zigbee RUC Window Covering**

***

## Introduction

This implementation can be seen as a simple extension of the Wiki: Zigbee RUC Lighting, adding another Zigbee cluster for the Home automation aspect.
The project generates a Window Device as Router (**ZR**) and a Window Controller as End Device (**ZED**) for Window covering features.


For more details, please refer to [Wiki Zigbee RUC Window Covering](https://wiki.st.com/stm32mcu/wiki/Connectivity:Zigbee_Realistic_Use_Case_Window_Covering)

## Application description

This example uses the **STM32WB5MM-DK board** as a roller shutter device and the **P-NUCLEO-WB55 board** as a roller shutter controller.
After binding the roller shutter with the controller, you can control the movement of the roller shutter.

For more information on the boards, please visit [P-NUCLEO-WB55 Pack](https://www.st.com/en/evaluation-tools/p-nucleo-wb55.html) and [STM32WB5MM-DK Pack](https://www.st.com/en/evaluation-tools/stm32wb5mm-dk.html)

## Setup

### Software requirements

The following applications are running on the **P-NUCLEO-WB55 boards**:

- Zigbee_Coord
- Zigbee_Shutter_Remote

The following application run on the **STM32WB5MM-DK board**:

- Zigbee_Roller_Shutter

The applications ***Zigbee_Coord*** and ***Zigbee_Roller_Shutter*** require having the *stm32wb5x_Zigbee_FFD_fw.bin* binary to be flashed on the Wireless Coprocessor. All other applications require having the *stm32wb5x_Zigbee_RFD_fw.bin* binary. If it is not the case, you need to use the STM32CubeProgrammer to load the appropriate binary.
For the detailed procedure to the Wireless Coprocessor binary see following wiki for hardware setup [STM32WB Build Zigbee project](https://wiki.st.com/stm32mcu/wiki/Connectivity:STM32WB_Build_Zigbee_Project)

In order to make the programs work, you must do the following:

- Open your toolchain
- Rebuild all files and flash the board with the executable file

For more details refer to the Application Note :  [AN5289 - Building a Wireless application](https://www.st.com/resource/en/application_note/an5289-how-to-build-wireless-applications-with-stm32wb-mcus-stmicroelectronics.pdf)

### Hardware requirements

For ***Zigbee_Roller_Shutter*** application, it uses an Arduino Motor Shield (AMS) based on the ST dual full-bridge L298P [Arduino Motor Shield](https://docs.arduino.cc/hardware/motor-shield-rev3/)

### Install the Zigbee Network

To install the RUC example, please follow the [HowTo Join a Zigbee Network](https://wiki.st.com/stm32mcu/wiki/Connectivity:Introduction_to_Zigbee_Realistic_Use_Case#HowTo_Join_Zigbee_Network) and [HowTo bind devices](https://wiki.st.com/stm32mcu/wiki/Connectivity:Introduction_to_Zigbee_Realistic_Use_Case#HowTo_bind_devices)

## Troubleshooting

**Caution** : Issues and the pull-requests are **not supported** to submit problems or suggestions related to the software delivered in this repository. The STM32WB-ZB-RUC Window Covering example is being delivered as-is, and not necessarily supported by ST.

**For any other question** related to the product, the hardware performance or characteristics, the tools, the environment, you can submit it to the **ST Community** on the STM32 MCUs related [page](https://community.st.com/s/topic/0TO0X000000BSqSWAW/stm32-mcus).
