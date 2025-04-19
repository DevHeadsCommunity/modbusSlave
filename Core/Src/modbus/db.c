#include "db.h"

void (*sendToSerial)(uint8_t *response, uint16_t responseLength) = NULL;

/**
 * @brief Initialize the ModbusSlaveData structure with the given parameters.
 *
 * @param data Pointer to the ModbusSlaveData structure.
 * @param id Slave ID.
 * @param numHoldingRegs Number of holding registers.
 * @param numInputRegs Number of input registers.
 * @param numCoils Number of coils.
 * @param numDiscreteInputs Number of discrete inputs.
 * @return int 0 on success, -1 on failure.
 */
int initModbusSlaveData(ModbusSlaveData *data, uint8_t id, size_t numHoldingRegs, size_t numInputRegs, size_t numCoils, size_t numDiscreteInputs)
{
    data->slaveID = id;
    data->numHoldingRegs = numHoldingRegs;
    data->numInputRegs = numInputRegs;
    data->numCoils = numCoils;
    data->numDiscreteInputs = numDiscreteInputs;

    data->holdingRegisters = (uint16_t *)calloc(numHoldingRegs, sizeof(uint16_t));
    if (data->holdingRegisters == NULL)
    {
        return -1;
    }

    data->discreteInputs = (uint8_t *)calloc((numDiscreteInputs + 7) / 8, sizeof(uint8_t));
    if (data->discreteInputs == NULL)
    {
        free(data->holdingRegisters);
        free(data->inputRegisters);
        free(data->coils);
        return -1;
    }

    return 0;
}

// Free the memory allocated for the ModbusSlaveData structure
void freeModbusSlaveData(ModbusSlaveData *data)
{
    free(data->holdingRegisters);
    free(data->inputRegisters);
    free(data->coils);
    free(data->discreteInputs);
}


// Access the nth discrete input (0-based index) with boundary check
// Returns the state of the discrete input (0 or 1)
bool dbGetDiscreteInputState(uint8_t *discreteInputs, size_t n)
{
    size_t byteIndex = n / 8;
    size_t bitIndex = n % 8;
    return (discreteInputs[byteIndex] & (1 << bitIndex)) != 0;
}

// Set the nth discrete input (0-based index) with boundary check
// Sets the state of the discrete input to the given value (0 or 1)
void dbSetDiscreteInputState(uint8_t *discreteInputs, size_t n, bool state)
{
    size_t byteIndex = n / 8;
    size_t bitIndex = n % 8;
    if (state)
    {
        discreteInputs[byteIndex] |= (1 << bitIndex); // Set the bit
    }
    else
    {
        discreteInputs[byteIndex] &= ~(1 << bitIndex); // Clear the bit
    }
}

// Access the nth holding register (0-based index) with boundary check
// Returns the value of the holding register
uint16_t dbGetHoldingRegister(uint16_t *holdingRegisters, size_t n)
{
    return holdingRegisters[n];
}

// Set the nth holding register (0-based index) with boundary check
// Sets the value of the holding register to the given value
void dbSetHoldingRegister(uint16_t *holdingRegisters, size_t n, uint16_t value)
{
    holdingRegisters[n] = value;
}

// Function to get the state of a discrete input with boundary check
// Returns TRUE if successful, FALSE if the address is out of bounds
bool getDiscreteInputState(ModbusSlaveData *data, size_t n, bool *state)
{
    if (n < DISCRETE_INPUTS_START_ADDRESS || n - DISCRETE_INPUTS_START_ADDRESS >= data->numDiscreteInputs)
    {
        return FALSE; // Boundary condition error
    }
    *state = dbGetDiscreteInputState(data->discreteInputs, n - DISCRETE_INPUTS_START_ADDRESS);
    return TRUE; // Success
}

// Function to get the value of a holding register with boundary check
// Returns TRUE if successful, FALSE if the address is out of bounds
bool getHoldingRegister(ModbusSlaveData *data, size_t n, uint16_t *value)
{
    if (n < HOLDING_REGISTERS_START_ADDRESS || n - HOLDING_REGISTERS_START_ADDRESS >= data->numHoldingRegs)
    {
        return FALSE; // Boundary condition error
    }
    *value = dbGetHoldingRegister(data->holdingRegisters, n - HOLDING_REGISTERS_START_ADDRESS);
    return TRUE; // Success
}

// Function to set the value of a holding register with boundary check
// Returns TRUE if successful, FALSE if the address is out of bounds
bool setHoldingRegister(ModbusSlaveData *data, size_t n, uint16_t value)
{
    if (n < HOLDING_REGISTERS_START_ADDRESS || n - HOLDING_REGISTERS_START_ADDRESS >= data->numHoldingRegs)
    {
        return FALSE; // Boundary condition error
    }
    dbSetHoldingRegister(data->holdingRegisters, n - HOLDING_REGISTERS_START_ADDRESS, value);
    return TRUE; // Success
}

// Function to set the state of a discrete input with boundary check
// Returns TRUE if successful, FALSE if the address is out of bounds
bool setDiscreteInputState(ModbusSlaveData *data, size_t n, bool state)
{
    if (n < DISCRETE_INPUTS_START_ADDRESS || n - DISCRETE_INPUTS_START_ADDRESS >= data->numDiscreteInputs)
    {
        return FALSE; // Boundary condition error
    }
    dbSetDiscreteInputState(data->discreteInputs, n - DISCRETE_INPUTS_START_ADDRESS, state);
    return TRUE; // Success
}

