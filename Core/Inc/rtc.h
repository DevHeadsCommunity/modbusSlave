/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   This file contains all the function prototypes for
  *          the rtc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */
typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint16_t SubSeconds;
	uint8_t date;
	uint8_t month;
	uint8_t year;
	uint8_t day;	//actual day of week
}rtcTime;
/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
void DS1307_GetDate(uint8_t *dayOfWeek, uint8_t *date, uint8_t *month, uint8_t *year) ;
void DS1307_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
void DS1307_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
void DS1307_SetDate(uint8_t dayOfWeek, uint8_t date, uint8_t month, uint8_t year);
void readRTC(rtcTime *newTime);
void setRTC( rtcTime* newTime );
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

