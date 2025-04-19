#include "modbusMaster.h"
#include "main.h"


// Function to calculate the length of the Modbus frame at the master end
uint16_t getModMasterFrameLen(const uint8_t *frame)
{
	 uint8_t function_code = frame[1];

	    switch (function_code) {
	        case 1: case 2: { // Read Coils, Read Discrete Inputs
	            uint8_t byte_count = frame[2];
	            return 5 + byte_count;
	        }
	        case 3: case 4: { // Read Holding/Input Registers
	            uint8_t byte_count = frame[2];
	            return 5 + byte_count;
	        }
	        case 5: case 6: // Write Single Coil/Register
	            return 8;
	        case 15: case 16: // Write Multiple Coils/Registers
	            return 8;
	        default:
	            return -1; // Unsupported function code
	    }
}

// Function to process a Modbus response frame
// response: Pointer to the response frame buffer
// responseLength: Length of the response frame
// Returns: slave address if the response is successfully processed, 0 otherwise
uint8_t modbusProcessResponse(uint8_t *response, uint16_t responseLength, uint8_t *values, uint16_t *countOrAddress)
{
    if (responseLength < 5) 
    {
        // Minimum length for a valid Modbus response is 5 bytes
        printDebug("Invalid response length\n");
        return 0;
    }


    uint8_t slaveAddress = response[0];
    uint8_t functionCode = response[1];
    uint16_t crcReceived = (uint16_t)response[responseLength - CRC_LENGTH] | ((uint16_t)response[responseLength - 1] << 8);
    uint16_t crcCalculated = modbus_crc16(response, responseLength - 2);

    printDebug("receivedCRC=%04X calculatedCRC=%04X\n", crcReceived, crcCalculated);
    
    if (crcReceived != crcCalculated) 
    {
        // CRC check failed
        printDebug("CRC check failed\n");
        return 0;
    }

    printDebug("functionCode=%d\n", functionCode);

    // Process the response based on the function code
    switch (functionCode)
    {

    case 0x02: // Read Discrete Inputs
        // Extract the byte count
        countOrAddress = response[2];
        memcpy(values, response + 3,countOrAddress);
        break;
        
    case 0x10: // Write Multiple Registers

        // Extract the address
        countOrAddress = response[2] | (response[3] << 8);
        memcpy(values, response + 3,2); // Skip address and send quantity fields pointer
        break;
        
    default:
        // Unsupported function code
        return 0;
    }

    return slaveAddress; // Indicate successful processing
}




// Function to form a Modbus request frame
// frame: Pointer to the frame buffer
// slaveID: Slave ID
// functionCode: Function code
// startAddress: Starting address
// numItems: Number of items (registers or coils)
// values: Array of values (for write operations)
static uint16_t form_modbus_request_frame(uint8_t *frame, uint8_t slaveID, uint8_t functionCode, uint16_t startAddress, uint16_t numItems, const uint8_t *values)
{
    frame[0] = slaveID;                    // Slave ID
    frame[1] = functionCode;               // Function code
    frame[2] = (startAddress >> 8) & 0xFF; // Start address high byte
    frame[3] = startAddress & 0xFF;        // Start address low byte

    uint16_t frameLength = 0;
    uint16_t crc;
   
    if (functionCode == WRITE_MULTIPLE_REGISTERS)
    {
        frame[4] = (numItems >> 8) & 0xFF; // Number of items high byte
        frame[5] = numItems & 0xFF;        // Number of items low byte
        frame[6] = numItems * 2;           // Byte count for register values

        // Add the register values to the frame
        for (uint16_t i = 0; i < numItems; i++)
        {
            frame[7 + (i * 2)] = values[i * 2 + 1];     // Register value high byte
            frame[8 + (i * 2)] = values[i * 2]; // Register value low byte
        }

        // Calculate the CRC for the frame
        crc = modbus_crc16(frame, 7 + (numItems * 2));
        frame[7 + (numItems * 2)] = crc & 0xFF;        // CRC low byte
        frame[8 + (numItems * 2)] = (crc >> 8) & 0xFF; // CRC high byte

        frameLength = 9 + (numItems * 2);
    }

    else
    {
        // For read operations
        frame[4] = (numItems >> 8) & 0xFF; // Number of items high byte
        frame[5] = numItems & 0xFF;        // Number of items low byte

        // Calculate the CRC for the frame
        crc = modbus_crc16(frame, 6);
        frame[6] = crc & 0xFF;        // CRC low byte
        frame[7] = (crc >> 8) & 0xFF; // CRC high byte

        frameLength = 8;
    }

    printDebug("crc=%04X\n", crc);

    return frameLength;
}



void writeRegisters(uint8_t slaveID, uint16_t address, uint16_t *values, uint16_t numRegisters)
{
    uint8_t frame[40] = {0};
    uint8_t frameLen = form_modbus_request_frame(frame, slaveID, WRITE_MULTIPLE_REGISTERS, address - HOLDING_REGISTERS_START_ADDRESS, numRegisters, (uint8_t *)values);
    RS485_Transmit(frame, frameLen);
}

void requestDiscreteInputs(uint8_t slaveID, uint16_t address, uint16_t numInputs)
{
    uint8_t frame[16] = {0};
    uint8_t frameLen = form_modbus_request_frame(frame, slaveID, READ_DISCRETE_INPUTS, address - DISCRETE_INPUTS_START_ADDRESS, numInputs, NULL);
    RS485_Transmit(frame, frameLen);
}
