#ifndef DB_H
#define DB_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define COILS_START_ADDRESS 00001
#define DISCRETE_INPUTS_START_ADDRESS 10001
#define INPUT_REGISTERS_START_ADDRESS 30001
#define HOLDING_REGISTERS_START_ADDRESS 40001

// Define boolean alternatives for cross-platform compatibility
#define TRUE 1
#define FALSE 0

typedef struct
{
    uint8_t slaveID;
    uint16_t *holdingRegisters;
    uint16_t *inputRegisters;
    uint8_t *coils;
    uint8_t *discreteInputs;
    size_t numHoldingRegs;
    size_t numInputRegs;
    size_t numCoils;
    size_t numDiscreteInputs;
} ModbusSlaveData;

int initModbusSlaveData(ModbusSlaveData *data, uint8_t id, size_t numHoldingRegs, size_t numInputRegs, size_t numCoils, size_t numDiscreteInputs);

void freeModbusSlaveData(ModbusSlaveData *data);

bool dbGetDiscreteInputState(uint8_t *discreteInputs, size_t n);

void dbSetDiscreteInputState(uint8_t *discreteInputs, size_t n, bool state);

uint16_t dbGetHoldingRegister(uint16_t *holdingRegisters, size_t n);

void dbSetHoldingRegister(uint16_t *holdingRegisters, size_t n, uint16_t value);

bool getDiscreteInputState(ModbusSlaveData *data, size_t n, bool *state);

bool getHoldingRegister(ModbusSlaveData *data, size_t n, uint16_t *value);

bool setHoldingRegister(ModbusSlaveData *data, size_t n, uint16_t value);

bool setDiscreteInputState(ModbusSlaveData *data, size_t n, bool state);

#endif // DB_H
