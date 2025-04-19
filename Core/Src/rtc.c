/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */



void setRTC( rtcTime* newTime )
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	sTime.Hours = newTime->hours;
	sTime.Minutes = newTime->minutes;
	sTime.Seconds = newTime->seconds;
	sTime.SubSeconds = newTime->SubSeconds;
	sTime.SecondFraction = 1000;

	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;

	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}

	sDate.WeekDay = newTime->day;
	sDate.Month = newTime->month;
	sDate.Date = newTime->date;
	sDate.Year = newTime->year;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}

void readRTC(rtcTime *newTime)
{
	RTC_TimeTypeDef stimestructureget;
	  RTC_DateTypeDef sdatestructureget;
	uint32_t subseconds, secondfraction;

	  /* Get the RTC current Time */
	  HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);

	  /*Date is not required but without reading data its returning
	   * time value = 0 */

	  /* Get the RTC current Date */
	  HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);


	subseconds = stimestructureget.SubSeconds;
	secondfraction = hrtc.Init.SynchPrediv;

	// Calculate the exact time in milliseconds
	uint32_t milliseconds = ((secondfraction - subseconds) * 1000) / (secondfraction + 1);

	newTime->SubSeconds = milliseconds;
	newTime->seconds = stimestructureget.Seconds;
	newTime->minutes = stimestructureget.Minutes;
	newTime->hours = stimestructureget.Hours;

	newTime->date = sdatestructureget.Date;
	newTime->month = sdatestructureget.Month;
	newTime->year = sdatestructureget.Year;
	newTime->day = sdatestructureget.WeekDay;

}


//external RTC

#define DS1307_ADDRESS 0xD0  // 7-bit address shifted for HAL

extern I2C_HandleTypeDef hi2c1;

// Convert Decimal to Binary-Coded Decimal (BCD)
uint8_t DecimalToBCD(uint8_t val) {
	return ((val / 10) << 4) | (val % 10);
}
uint8_t BCDToDecimal(uint8_t val) {
	return ((val >> 4) * 10) + (val & 0x0F);
}

void DS1307_SetDate(uint8_t dayOfWeek, uint8_t date, uint8_t month, uint8_t year) {
	uint8_t buffer[5];

	// Prepare data for date registers
	buffer[0] = 0x03;                     // Start at register 0x03 (day of the week)
	buffer[1] = DecimalToBCD(dayOfWeek);  // Day of the week (1 = Sunday, 7 = Saturday)
	buffer[2] = DecimalToBCD(date);       // Date
	buffer[3] = DecimalToBCD(month);      // Month
	buffer[4] = DecimalToBCD(year);       // Year (last two digits)

	// Write to DS1307
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_ADDRESS, buffer, 5, HAL_MAX_DELAY);
}

void DS1307_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
	uint8_t buffer[4];

	// Prepare data for time registers
	buffer[0] = 0x00;                        // Start at register 0x00 (seconds)
	buffer[1] = DecimalToBCD(seconds);       // Seconds (CH bit is cleared automatically)
	buffer[2] = DecimalToBCD(minutes);       // Minutes
	buffer[3] = DecimalToBCD(hours);         // Hours

	// Write to DS1307
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_ADDRESS, buffer, 4, HAL_MAX_DELAY);
}


void DS1307_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
	uint8_t buffer[3];
	uint8_t reg = 0x00;  // Start at register 0x00 (seconds)

	// Request data from DS1307 starting at seconds register
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_ADDRESS, &reg, 1, HAL_MAX_DELAY);

	// Read 3 bytes (seconds, minutes, hours)
	HAL_I2C_Master_Receive(&hi2c1, DS1307_ADDRESS, buffer, 3, HAL_MAX_DELAY);

	// Convert BCD to Decimal
	*seconds = BCDToDecimal(buffer[0] & 0x7F);  // Mask CH bit (bit 7)
	*minutes = BCDToDecimal(buffer[1]);
	*hours = BCDToDecimal(buffer[2] & 0x3F);    // Mask 24-hour mode bits
}

void DS1307_GetDate(uint8_t *dayOfWeek, uint8_t *date, uint8_t *month, uint8_t *year) {
	uint8_t buffer[4];
	uint8_t reg = 0x03;  // Start at register 0x03 (day of the week)

	// Request data from DS1307 starting at day of the week register
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_ADDRESS, &reg, 1, HAL_MAX_DELAY);

	// Read 4 bytes (day of the week, date, month, year)
	HAL_I2C_Master_Receive(&hi2c1, DS1307_ADDRESS, buffer, 4, HAL_MAX_DELAY);

	// Convert BCD to Decimal
	*dayOfWeek = BCDToDecimal(buffer[0]);
	*date = BCDToDecimal(buffer[1]);
	*month = BCDToDecimal(buffer[2]);
	*year = BCDToDecimal(buffer[3]);  // Last two digits of the year
}

/* USER CODE END 1 */
