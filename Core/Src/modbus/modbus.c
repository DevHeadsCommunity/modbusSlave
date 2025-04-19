#include "modbus.h"
#include "main.h"



// Function to calculate the CRC16 for Modbus RTU
// data: Pointer to the data array
// length: Length of the data array
// Returns the calculated CRC16 value
 uint16_t modbus_crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF; // Initialize CRC to 0xFFFF

    // Loop through each byte in the data array
    for (uint16_t pos = 0; pos < length; pos++)
    {
        crc ^= (uint16_t)data[pos]; // XOR byte into least significant byte of CRC

        // Loop through each bit in the current byte
        for (int i = 8; i != 0; i--)
        {
            // If the least significant bit is set
            if ((crc & 0x0001) != 0)
            {
                crc >>= 1;     // Shift right
                crc ^= 0xA001; // XOR with polynomial 0xA001
            }
            else
            {
                crc >>= 1; // Just shift right
            }
        }
    }
    return crc; // Return the calculated CRC16 value
}




// Function to generate a Modbus exception response
// frame: Pointer to the Modbus frame
// length: Length of the Modbus frame
// exceptionCode: The Modbus exception code
// response: Pointer to the response buffer
// responseLength: Pointer to the length of the response buffer
static void generateModbusException(const uint8_t *frame, uint8_t exceptionCode, uint8_t *response, uint16_t *responseLength)
{
    // Prepare the response frame
    response[0] = frame[0];        // Slave ID
    response[1] = frame[1] | 0x80; // Function code with high bit set to indicate exception
    response[2] = exceptionCode;   // Exception code

    // Calculate the CRC for the response
    uint16_t crc = modbus_crc16(response, 3);
    response[3] = crc & 0xFF;        // CRC low byte
    response[4] = (crc >> 8) & 0xFF; // CRC high byte

    printDebug("crc=%04X\n", crc);

    // Set the response length
    *responseLength = 5;
}

static bool handleReadDiscreteInputs(const uint8_t *frame, ModbusSlaveData *slave, uint8_t *response, uint16_t *responseLength)
{
    uint16_t startAddress = (uint16_t)frame[2] << 8 | (uint16_t)frame[3];
    uint16_t quantityOfInputs = (uint16_t)frame[4] << 8 | (uint16_t)frame[5];

    if ((startAddress + quantityOfInputs) > slave->numDiscreteInputs)
    {
        generateModbusException(frame, ILLEGAL_DATA_ADDRESS, response, responseLength);
        return FALSE;
    }

    response[0] = slave->slaveID;
    response[1] = frame[1];
    response[2] = (quantityOfInputs + 7) / 8;

    for (uint16_t i = 0; i < quantityOfInputs; i++)
    {
        bool state;
        state = dbGetDiscreteInputState(slave->discreteInputs, startAddress + i);
        if (state)
        {
            response[3 + (i / 8)] |= (1 << (i % 8));
        }
        else
        {
            response[3 + (i / 8)] &= ~(1 << (i % 8));
        }
    }

    uint16_t crc = modbus_crc16(response, RESPONSE_HEADER_LENGTH + response[BYTE_COUNT_OFFSET]);
    response[CRC_LOW_BYTE_OFFSET + response[BYTE_COUNT_OFFSET]] = crc & 0xFF;
    response[CRC_HIGH_BYTE_OFFSET + response[BYTE_COUNT_OFFSET]] = (crc >> 8) & 0xFF;

    printDebug("crc=%04X\n", crc);

    *responseLength = EXCEPTION_RESPONSE_LENGTH + response[BYTE_COUNT_OFFSET];
    return TRUE;
}

static bool handleReadHoldingRegisters(const uint8_t *frame, ModbusSlaveData *slave, uint8_t *response, uint16_t *responseLength)
{
    uint16_t startAddress = (uint16_t)frame[2] << 8 | (uint16_t)frame[3];
    uint16_t quantityOfRegisters = (uint16_t)frame[4] << 8 | (uint16_t)frame[5];

    if ((startAddress + quantityOfRegisters) > slave->numHoldingRegs)
    {
        generateModbusException(frame, ILLEGAL_DATA_ADDRESS, response, responseLength);
        return FALSE;
    }

    response[0] = slave->slaveID;
    response[1] = frame[1];
    response[2] = quantityOfRegisters * 2;

    for (uint16_t i = 0; i < quantityOfRegisters; i++)
    {
        uint16_t value;
        value = dbGetHoldingRegister(slave->holdingRegisters, startAddress + i);
        response[3 + (i * 2)] = (value >> 8) & 0xFF;
        response[4 + (i * 2)] = value & 0xFF;
    }

    uint16_t crc = modbus_crc16(response, RESPONSE_HEADER_LENGTH + response[BYTE_COUNT_OFFSET]);
    response[CRC_LOW_BYTE_OFFSET + response[BYTE_COUNT_OFFSET]] = crc & 0xFF;
    response[CRC_HIGH_BYTE_OFFSET + response[BYTE_COUNT_OFFSET]] = (crc >> 8) & 0xFF;

    printDebug("crc=%04X\n", crc);

    *responseLength = EXCEPTION_RESPONSE_LENGTH + response[BYTE_COUNT_OFFSET];
    return TRUE;
}

static bool handleWriteMultipleRegisters(const uint8_t *frame, ModbusSlaveData *slave, uint8_t *response, uint16_t *responseLength)
{
    uint16_t startAddress = (uint16_t)frame[2] << 8 | (uint16_t)frame[3];
    uint16_t quantityOfRegisters = (uint16_t)frame[4] << 8 | (uint16_t)frame[5];
    uint8_t byteCount = frame[6];

    if ((startAddress + quantityOfRegisters) > slave->numHoldingRegs)
    {
        generateModbusException(frame, ILLEGAL_DATA_ADDRESS, response, responseLength);
        return FALSE;
    }

    for (uint16_t i = 0; i < quantityOfRegisters; i++)
    {
        uint16_t value = (uint16_t)frame[7 + (i * 2)] << 8 | (uint16_t)frame[8 + (i * 2)];
        dbSetHoldingRegister(slave->holdingRegisters, startAddress + i, value);
    }

    for (uint16_t i = 0; i < 6; i++)
    {
        response[i] = frame[i];
    }

    uint16_t crc = modbus_crc16(response, 6);
    response[6] = crc & 0xFF;
    response[7] = (crc >> 8) & 0xFF;

    printDebug("crc=%04X\n", crc);

    *responseLength = 8;

        slaveCallback(startAddress + HOLDING_REGISTERS_START_ADDRESS, quantityOfRegisters);
    return TRUE;
}




// Function to calculate the length of the Modbus frame
uint16_t getModSlaveFrameLen(const uint8_t *frame)
{
	   uint8_t function_code = frame[1];

	    switch (function_code) {
	        case 1: case 2: case 3: case 4: // Read Coils, Inputs, Holding Registers, Input Registers
	            return 8;
	        case 5: case 6: // Write Single Coil/Register
	            return 8;
	        case 15: case 16: { // Write Multiple Coils/Registers
	            uint8_t byte_count = frame[6];
	            return 9 + byte_count;
	        }
	        default:
	            return -1; // Unsupported function code
	    }
}

// Function to parse a Modbus frame
// frame: Pointer to the Modbus frame
// length: Length of the Modbus frame
// slave: Pointer to the ModbusSlaveData structure
// response: Pointer to the response buffer
// responseLength: Pointer to the length of the response buffer
// Returns TRUE if the frame is valid, FALSE otherwise
static bool parseModbusFrame(const uint8_t *frame, uint16_t length, ModbusSlaveData *slave, uint8_t *response, uint16_t *responseLength)
{
    ExceptionCodes exceptionCode = NO_EXCEPTION;

    // A valid Modbus frame must be at least 4 bytes long (address, function, data, CRC)
    if (length < MIN_MODBUS_FRAME_LENGTH)
    {
        printDebug("Invalid frame length, function code:%d\n", frame[1]);
        return FALSE;
    }

    // Check if the slave ID matches or if it's a broadcast message (slave ID 0)
    if (frame[0] != slave->slaveID && frame[0] != MODBUS_BROADCAST_ADDRESS)
    {
        printDebug("Invalid slave ID, function code:%d\n", frame[1]);
        return FALSE;
    }

    // Calculate the CRC of the frame excluding the last two bytes (CRC)
    uint16_t calculatedCrc = modbus_crc16(frame, length - CRC_LENGTH);

    // Extract the CRC from the frame (last two bytes)
    uint16_t receivedCrc = (uint16_t)frame[length - CRC_LENGTH] | ((uint16_t)frame[length - 1] << 8);

    printDebug("receivedCRC=%04X calculatedCRC=%04X\n", receivedCrc, calculatedCrc);

    // Check if the calculated CRC matches the received CRC
    if (calculatedCrc != receivedCrc)
    {
        printDebug("CRC check failed, function code:%d\n",frame[1]);
        return FALSE;
    }

    // Extract the function code from the frame (second byte)
    uint8_t functionCode = frame[1];

    // Handle the function code
    switch (functionCode)
    {
    case READ_DISCRETE_INPUTS:
        return handleReadDiscreteInputs(frame, slave, response, responseLength);
    case READ_HOLDING_REGISTERS:
        return handleReadHoldingRegisters(frame, slave, response, responseLength);
    case WRITE_MULTIPLE_REGISTERS:
        return handleWriteMultipleRegisters(frame, slave, response, responseLength);
    default:
        exceptionCode = ILLEGAL_FUNCTION;
        generateModbusException(frame, exceptionCode, response, responseLength);
        return FALSE;
    }
}


// Function to handle a Modbus request
// frame: Pointer to the Modbus frame
// length: Length of the Modbus frame
// slave: Pointer to the ModbusSlaveData structure
// response: Pointer to the response buffer
// responseLength: Pointer to the length of the response buffer
// Returns TRUE if the request is handled, FALSE otherwise
bool handleModbusRequest(const uint8_t *frame, uint16_t length, ModbusSlaveData *slave)
{
    uint8_t response[128] = {0};
    uint16_t responseLength = 0;

    // Parse the Modbus frame
    if (parseModbusFrame(frame, length, slave, response, &responseLength))
    {
        // Write frame to serial
        printDebug("Modbus frame processed\n");
        if(response[0] != MODBUS_BROADCAST_ADDRESS)
        {
            RS485_Transmit(response, responseLength);
        }

        return TRUE; // Frame is valid and response is prepared
    }
    else
    {
        // Write exception response to serial
        if ((response[0] != MODBUS_BROADCAST_ADDRESS) && (responseLength > 0))
        {
            RS485_Transmit(response, responseLength);
        }
        
        return FALSE; // Frame is invalid or error occurred
    }
}

