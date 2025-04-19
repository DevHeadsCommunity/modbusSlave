/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "usart.h"
#include "rtc.h"
#include "i2c.h"

#include "modbus/config.h"
#include "modbus/modbus.h"
#include "modbus/modbusMaster.h"
#include "oled/ssd1306.h"
#include "oled/ssd1306_fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* In master mode :To store event time from two slaves
 * In slave mode  : store own event time stamp on 0th index
 */
rtcTime eventTime[2];

/* USER CODE BEGIN 4 */

#define MODBUS_RTU_SILENCE_TIME  2 // 2 ticks (2 ms)


// Buffer for received data
#define RX_BUFFER_SIZE 256 // Buffer size for receiving data
typedef struct uartStream
{
	uint8_t rxBuffer[RX_BUFFER_SIZE];
	uint8_t txBuffer[RX_BUFFER_SIZE];
	uint8_t rxByte;
	uint8_t rxFillIndex;
	uint8_t txFillIndex;
	uint8_t rxReadIndex;
	uint8_t txReadIndex;
	uint32_t lastByteTimestamp;
} uartStream;

uartStream uart1Stream;
uartStream uart2Stream;

/* USER CODE END Variables */
osThreadId lcdTaskHandle;
osThreadId modbusTaskHandle;
osThreadId consoleHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void lcdHandlerTask(void const * argument);
void ModbusSlaveTask(void const * argument);
void consoleTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of lcdTask */
	osThreadDef(lcdTask, lcdHandlerTask, osPriorityNormal, 0, 400);
	lcdTaskHandle = osThreadCreate(osThread(lcdTask), NULL);

	/* definition and creation of modbusTask */
	osThreadDef(modbusTask, ModbusSlaveTask, osPriorityHigh, 0, 512);
	modbusTaskHandle = osThreadCreate(osThread(modbusTask), NULL);

	/* definition and creation of console */
	osThreadDef(console, consoleTask, osPriorityIdle, 0, 256);
	consoleHandle = osThreadCreate(osThread(console), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_lcdHandlerTask */
/**
 * @brief  Function implementing the lcdTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_lcdHandlerTask */
void lcdHandlerTask(void const * argument)
{
	/* USER CODE BEGIN lcdHandlerTask */
	char textBuffer[32];
	rtcTime time;
	time.SubSeconds = 0;

	ssd1306_Init();
	ssd1306_Fill(Black);
	ssd1306_DrawRectangle(0,0,127,63,1);
	ssd1306_UpdateScreen();

	// Read the date
	DS1307_GetDate(&time.day, &time.date, &time.month, &time.year);
	//read time
	DS1307_GetTime(&time.hours, &time.minutes, &time.seconds);

	setRTC(&time);

	/* Infinite loop */
	for(;;)
	{
		readRTC(&time);


		ssd1306_SetCursor(16, 6);
		sprintf(textBuffer, "Time %.2d:%.2d:%.2d.%.3d", time.hours,
				time.minutes, time.seconds, time.SubSeconds);
		ssd1306_WriteString(textBuffer, Font_6x8, White);

		ssd1306_SetCursor(10, 26);
		sprintf(textBuffer, "Event %.2d:%.2d:%.2d.%.3d", eventTime[0].hours,
				eventTime[0].minutes, eventTime[0].seconds,
				eventTime[0].SubSeconds);
		ssd1306_WriteString(textBuffer, Font_6x8, White);


		setHoldingRegister(&modbusSlave, RTC_HOURS, (uint16_t)time.hours);
		setHoldingRegister(&modbusSlave, RTC_MINUTES, (uint16_t)time.minutes);
		setHoldingRegister(&modbusSlave, RTC_SECONDS, (uint16_t)time.seconds);
		setHoldingRegister(&modbusSlave, RTC_SUBSECONDS, (uint16_t)time.SubSeconds);
		setHoldingRegister(&modbusSlave, RTC_DATE, (uint16_t)time.date);
		setHoldingRegister(&modbusSlave, RTC_MONTH, (uint16_t)time.month);
		setHoldingRegister(&modbusSlave, RTC_YEAR, (uint16_t)time.year);


		ssd1306_UpdateScreen();

		osDelay(50);
	}
	/* USER CODE END lcdHandlerTask */
}

/* USER CODE BEGIN Header_ModbusSlaveTask */

/*
 * @brief  Function to print debug messages to UART2.
 * @param  format: Format string (similar to printf).
 * @retval None
 */
void myPrintf(uint8_t debugLevel, const char *format, ...)
{
	if (debugLevel <= DEBUG_LEVEL)
	{
		char buffer[128];
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		// Ensure the buffer does not overflow
		size_t length = strlen(buffer);
		size_t index = 0;
		while (length)
		{
			uart2Stream.txBuffer[uart2Stream.txFillIndex++] = buffer[index++];
			if (uart2Stream.txFillIndex >= RX_BUFFER_SIZE)
			{
				uart2Stream.txFillIndex = 0;  // Reset index on overflow
			}
			length--;
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_3)
	{
		// Button pressed
		uint8_t buttonStatus = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);

		setDiscreteInputState(&modbusSlave, DI_buttonStatus, buttonStatus);
		readRTC(&eventTime[0]);
		printInfo("Button state=%d\t",buttonStatus);
		printInfo("TimeStamp= %.2d/%.2d/%.4d %.2d:%.2d:%.2d.%.3d\n",\
				eventTime[0].date,eventTime[0].month,eventTime[0].year+2000, \
				eventTime[0].hours,eventTime[0].minutes,eventTime[0].seconds,eventTime[0].SubSeconds);

	}
}




/// Interrupt callback when a byte is received
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uartStream *stream = NULL;

	if (huart->Instance == USART1)
	{
		stream = &uart1Stream;
	}
	else if (huart->Instance == USART2)
	{
		stream = &uart2Stream;
	}

	if (stream != NULL)
	{
		// Store received byte in the buffer directly from UART
		stream->rxBuffer[stream->rxFillIndex++] = stream->rxByte;

		stream->lastByteTimestamp = xTaskGetTickCount();

		// Prevent buffer overflow
		if (stream->rxFillIndex >= RX_BUFFER_SIZE)
		{
			stream->rxFillIndex = 0;  // Reset index on overflow
		}
		// Restart UART reception for the next byte
		HAL_UART_Receive_IT(huart, &stream->rxByte, 1);
	}
}


// Function to detect if a Modbus frame is ready for UART1
uint8_t isModbusFrameReady(uint8_t slaveID)
{
	uint8_t frameReady = 0;
	uartStream *stream = &uart1Stream;

	// Calculate elapsed time since last byte received
	uint32_t elapsedTime = xTaskGetTickCount() - stream->lastByteTimestamp;

	// Check if 3.5 character times have passed (frame is ready)
	if (elapsedTime >= MODBUS_RTU_SILENCE_TIME)
	{
		if((stream->rxFillIndex != stream->rxReadIndex))
		{
			frameReady = 1;  // Frame is ready for processing
		}
	}
	return frameReady;
}



void RS485_Transmit(uint8_t *data, uint16_t dataLen)
{
	// Set DE high and RE low (transmit mode)
	HAL_GPIO_WritePin(max485_DE_GPIO_Port, max485_DE_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(max485_RE_GPIO_Port, max485_RE_Pin, GPIO_PIN_SET);

	printInfo("sendLen=%d\n",dataLen);

	HAL_UART_Transmit(&huart1, data, dataLen, osWaitForever);

	// Set DE and RE low (receive mode) after transmission is complete
	HAL_GPIO_WritePin(max485_DE_GPIO_Port, max485_DE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(max485_RE_GPIO_Port, max485_RE_Pin, GPIO_PIN_RESET);
}



void slaveCallback(uint16_t regAddress, uint16_t numOfRegisters)
{
	switch (regAddress)
	{
	case RTC_DATE:
		if (numOfRegisters > RTC_DATE - RTC_SUBSECONDS)
		{
			regAddress += RTC_SUBSECONDS;

			uint16_t tempVal;
			rtcTime newTime;

			readRTC(&newTime);
			printInfo("old Time= %.2d/%.2d/%.4d %.2d:%.2d:%.2d.%.3d\n",
					newTime.date, newTime.month, newTime.year + 2000,
					newTime.hours, newTime.minutes, newTime.seconds, newTime.SubSeconds);

			if (getHoldingRegister(&modbusSlave, RTC_HOURS, &tempVal))
			{
				newTime.hours = tempVal;
			}
			if (getHoldingRegister(&modbusSlave, RTC_MINUTES, &tempVal))
			{
				newTime.minutes = tempVal;
			}
			if (getHoldingRegister(&modbusSlave, RTC_SECONDS, &tempVal))
			{
				newTime.seconds = tempVal;
			}
			if (getHoldingRegister(&modbusSlave, RTC_SUBSECONDS, &tempVal))
			{
				newTime.SubSeconds = tempVal;
			}

			if (getHoldingRegister(&modbusSlave, RTC_DATE, &tempVal))
			{
				newTime.date = tempVal;
			}
			if (getHoldingRegister(&modbusSlave, RTC_MONTH, &tempVal))
			{
				newTime.month = tempVal;
			}
			if (getHoldingRegister(&modbusSlave, RTC_YEAR, &tempVal))
			{
				newTime.year = tempVal - 2000;
			}

			DS1307_SetTime(newTime.hours, newTime.minutes, newTime.seconds);

			// Set the RTC date
			DS1307_SetDate(0, newTime.date, newTime.month, newTime.year);

			setRTC(&newTime);

			printInfo("New Time= %.2d/%.2d/%.4d %.2d:%.2d:%.2d.%.3d\n",
					newTime.date, newTime.month, newTime.year + 2000,
					newTime.hours, newTime.minutes, newTime.seconds, newTime.SubSeconds);
		}
		break;

	default:
		break;
	}
}



/**
 * @brief Function implementing the modbusTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_ModbusSlaveTask */
void ModbusSlaveTask(void const * argument)
{
	/* USER CODE BEGIN ModbusSlaveTask */

	initModbusSlaveData(&modbusSlave, slaveID, numOfHoldingRegs, numOfInputRegs, numOfCoils, numOfDisInput);


	//put max485 in reception mode
	HAL_GPIO_WritePin(max485_DE_GPIO_Port, max485_DE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(max485_RE_GPIO_Port, max485_RE_Pin, GPIO_PIN_RESET);

	// Start UART reception in interrupt mode
	HAL_UART_Receive_IT(&huart1, &uart1Stream.rxByte, 1);

	/* Infinite loop */
	for(;;)
	{
		// Example: Check if a complete Modbus frame has been received
		if (isModbusFrameReady(modbusSlave.slaveID))
		{
			uint8_t modlen = getModSlaveFrameLen(uart1Stream.rxBuffer+uart1Stream.rxReadIndex);

			printInfo("Request received=%d, modlen = %d\n", uart1Stream.rxFillIndex - uart1Stream.rxReadIndex,modlen);

			if(modlen <= uart1Stream.rxFillIndex - uart1Stream.rxReadIndex,modlen)
			{
				handleModbusRequest(uart1Stream.rxBuffer+uart1Stream.rxReadIndex, modlen, &modbusSlave);
				uart1Stream.rxReadIndex += modlen;
			}

			/* This can be improved as circular buffer is used for UART reception
			   if consumption in circular manner no need of reset */
			if((uart1Stream.rxFillIndex == uart1Stream.rxReadIndex))
			{
				uart1Stream.rxFillIndex = 0;  // Reset index for next frame
				uart1Stream.rxReadIndex = 0;  // Reset index for next frame
			}
			printInfo("received Modbus packet\n");

		}
		osDelay(5);
		/* USER CODE END ModbusSlaveTask */
	}
}
/* USER CODE BEGIN Header_consoleTask */
/**
 * @brief Function implementing the console thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_consoleTask */
void consoleTask(void const * argument)
{
	/* USER CODE BEGIN consoleTask */

	// Start UART reception in interrupt mode for UART2
	HAL_UART_Receive_IT(&huart2, &uart2Stream.rxByte, 1);

	for(;;)
	{
		// Check if data is received on UART2
		if (uart2Stream.rxFillIndex != uart2Stream.rxReadIndex)
		{
			// Parse received data
			char commandBuffer[32];
			uint8_t commandIndex = 0;

			while (uart2Stream.rxFillIndex != uart2Stream.rxReadIndex && commandIndex < sizeof(commandBuffer) - 1)
			{
				commandBuffer[commandIndex++] = uart2Stream.rxBuffer[uart2Stream.rxReadIndex++];
				if (uart2Stream.rxReadIndex >= RX_BUFFER_SIZE)
				{
					uart2Stream.rxReadIndex = 0;  // Reset index on overflow
				}
			}
			commandBuffer[commandIndex] = '\0';  // Null-terminate the command string

			// Check if the command is "setTime HH:MM:SS"
			if (strncmp(commandBuffer, "setTime ", 8) == 0)
			{
				RTC_TimeTypeDef newTime = {0};
				int hours, minutes, seconds;

				if (sscanf(commandBuffer + 8, "%2d:%2d:%2d", &hours, &minutes, &seconds) == 3)
				{
					// Set the RTC time
					newTime.Hours = hours;
					newTime.Minutes = minutes;
					newTime.Seconds = seconds;
					newTime.SubSeconds = 0;

					DS1307_SetTime(newTime.Hours, newTime.Minutes, newTime.Seconds);

					// Set the internal RTC time
					if (HAL_RTC_SetTime(&hrtc, &newTime, RTC_FORMAT_BIN) != HAL_OK)
					{
						printInfo("Failed to set internal RTC time\n");
					}
					else
					{
						printInfo("Time set to %.2d:%.2d:%.2d\n", newTime.Hours, newTime.Minutes, newTime.Seconds);
					}
				}
				else
				{
					printInfo("Invalid time format. Use HH:MM:SS\n");
				}
			}

			// Check if the command is "setDate DD/MM/YYYY"
			else if (strncmp(commandBuffer, "setDate ", 8) == 0)
			{
				RTC_DateTypeDef newDate = {0};
				int day, month, year;

				if (sscanf(commandBuffer + 8, "%2d/%2d/%4d", &day, &month, &year) == 3)
				{
					newDate.Date = day;
					newDate.Month = month;
					newDate.Year = year - 2000; // Adjust year for internal RTC

					// Set the RTC date
					DS1307_SetDate(0,newDate.Date, newDate.Month, newDate.Year);

					if (HAL_RTC_SetDate(&hrtc, &newDate, RTC_FORMAT_BIN) != HAL_OK)
					{
						printInfo("Failed to set internal RTC date\n");
					}
					else
					{
						printInfo("Date set to %.2d/%.2d/%.4d\n", newDate.Date, newDate.Month, newDate.Year + 2000);
					}
				}
				else
				{
					printInfo("Invalid date format. Use DD/MM/YYYY\n");
				}
			}
			else
			{
				printInfo("Unknown command: %s\n", commandBuffer);
			}
		}


		// Check if there is data to send from txBuffer
		while (uart2Stream.txFillIndex != uart2Stream.txReadIndex)
		{
			// Calculate the number of bytes to send

			// Send all available data from txBuffer
			HAL_UART_Transmit(&huart2, &uart2Stream.txBuffer[uart2Stream.txReadIndex], 1, HAL_MAX_DELAY);
			uart2Stream.txReadIndex++;

			// Prevent buffer overflow
			if (uart2Stream.txReadIndex >= RX_BUFFER_SIZE)
			{
				uart2Stream.txReadIndex = 0;  // Reset index on overflow
			}
		}

		osDelay(50);
	}
	/* USER CODE END consoleTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

