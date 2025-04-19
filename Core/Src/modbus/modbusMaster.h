#include "modbus.h"

uint8_t modbusProcessResponse(uint8_t *, uint16_t , uint8_t *, uint16_t *);

void writeRegisters(uint8_t slaveID, uint16_t address, uint16_t *values, uint16_t numRegisters);

void requestDiscreteInputs(uint8_t slaveID, uint16_t address, uint16_t numInputs);
