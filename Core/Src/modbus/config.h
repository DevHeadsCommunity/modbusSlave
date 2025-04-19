#ifndef CONFIG_H
#define CONFIG_H

#include "db.h"
#include "modbus.h"

const uint8_t slaveID = 1;

const size_t numOfHoldingRegs = 24;
const size_t numOfInputRegs = 16;
const size_t numOfCoils = 16;
const size_t numOfDisInput = 16;

//slave structure object
ModbusSlaveData modbusSlave;

// Modbus address definitions
enum modAddress
{
	RTC_DATE = HOLDING_REGISTERS_START_ADDRESS,
	RTC_MONTH ,
	RTC_YEAR ,
	RTC_DAY ,

	RTC_HOURS,
	RTC_MINUTES,
	RTC_SECONDS,
	RTC_SUBSECONDS,

	modbusLcdData,

	DI_buttonStatus = DISCRETE_INPUTS_START_ADDRESS,
};


#endif // MODBUS_SLAVE_H
