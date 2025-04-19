#ifndef MODBUS_H
#define MODBUS_H

#include "db.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define MODBUS_BROADCAST_ADDRESS 0x00
#define MIN_MODBUS_FRAME_LENGTH 4
#define CRC_LENGTH 2
#define RESPONSE_HEADER_LENGTH 3
#define EXCEPTION_RESPONSE_LENGTH 5
#define BYTE_COUNT_OFFSET 2
#define CRC_LOW_BYTE_OFFSET 3
#define CRC_HIGH_BYTE_OFFSET 4

typedef enum
{
    NO_EXCEPTION = 0x00,
    ILLEGAL_FUNCTION = 0x01,
    ILLEGAL_DATA_ADDRESS = 0x02,
    ILLEGAL_DATA_VALUE = 0x03,
    SLAVE_DEVICE_FAILURE = 0x04,
    ACKNOWLEDGE = 0x05,
    SLAVE_DEVICE_BUSY = 0x06,
    MEMORY_PARITY_ERROR = 0x08,
    GATEWAY_PATH_UNAVAILABLE = 0x0A,
    GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = 0x0B
} ExceptionCodes;

enum FunctionCodes
{
    READ_COILS = 0x01,
    READ_DISCRETE_INPUTS = 0x02,
    READ_HOLDING_REGISTERS = 0x03,
    READ_INPUT_REGISTERS = 0x04,
    WRITE_SINGLE_COIL = 0x05,
    WRITE_SINGLE_REGISTER = 0x06,
    WRITE_MULTIPLE_COILS = 0x0F,
    WRITE_MULTIPLE_REGISTERS = 0x10,
    REPORT_SLAVE_ID = 0x11
};

bool handleModbusRequest(const uint8_t *frame, uint16_t length, ModbusSlaveData *slave);

bool writeDataToHoldingRegisters(ModbusSlaveData *data, size_t startAddress, uint8_t *inputArray, size_t length);

bool readDataFromHoldingRegisters(ModbusSlaveData *data, size_t startAddress, uint8_t *outputArray, size_t length);

#endif // MODBUS_H
