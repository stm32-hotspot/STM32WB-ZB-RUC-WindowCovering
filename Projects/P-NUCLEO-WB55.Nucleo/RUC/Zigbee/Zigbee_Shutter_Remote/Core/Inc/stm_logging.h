/**
  ******************************************************************************
  * @file    stm_logging.h
  * @author  MCD Application Team
  * @brief   Application header file for logging
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

#ifndef STM_LOGGING_H_
#define STM_LOGGING_H_

#define LOG_LEVEL_NONE  0U  /* None     */
#define LOG_LEVEL_CRIT  1U  /* Critical */
#define LOG_LEVEL_WARN  2U  /* Warning  */
#define LOG_LEVEL_INFO  3U  /* Info     */
#define LOG_LEVEL_DEBG  4U  /* Debug    */

#define APP_DBG_FULL(level, region, ...)                                                    \
  {                                                                                         \
    if (APPLI_PRINT_FILE_FUNC_LINE == 1U)                                                   \
    {                                                                                       \
        printf("\r\n[%s][%s][%d] ", DbgTraceGetFileName(__FILE__),__FUNCTION__,__LINE__);   \
    }                                                                                       \
    logApplication(level, region, __VA_ARGS__);                                             \
  }

#define APP_DBG(...)                                                                        \
  {                                                                                         \
    if (APPLI_PRINT_FILE_FUNC_LINE == 1U)                                                   \
    {                                                                                       \
        printf("\r\n[%s][%s][%d] ", DbgTraceGetFileName(__FILE__),__FUNCTION__,__LINE__);   \
    }                                                                                       \
    logApplication(LOG_LEVEL_NONE, APPLI_LOG_REGION_GENERAL, __VA_ARGS__);                  \
  }

#define APP_ZB_DBG(...)                                                                 \
  {                                                                                     \
    char const * name = DbgTraceGetFileName(__FILE__);                                  \
    printf("[M4 APPLICATION] \x1b[38;5;%dm[",( (name[4] + name[5] * 8) % 115) + 117);   \
    for (int i=0; i < strlen(name) - 2; i++)                                            \
    {                                                                                   \
      if (name[i] >= 'a' && name[i] <= 'z')                                             \
      {                                                                                 \
        printf("%c", name[i] - ' ' );                                                   \
      }                                                                                 \
      else                                                                              \
      {                                                                                 \
        printf("%c", name[i]);                                                          \
      }                                                                                 \
   }                                                                                    \
   printf("]\x1B[m ");                                                                  \
   printf(__VA_ARGS__);                                                                 \
   printf("\n");                                                                        \
}

/**
 * This enumeration represents log regions.
 *
 */
typedef enum
{
  APPLI_LOG_REGION_GENERAL                    = 1U,  /* General                 */
  APPLI_LOG_REGION_ZIGBEE_API                 = 2U,  /* Zigbee API              */
} appliLogRegion_t;

typedef uint8_t appliLogLevel_t;

void logApplication(appliLogLevel_t aLogLevel, appliLogRegion_t aLogRegion, const char *aFormat, ...);

#endif  /* STM_LOGGING_H_ */
