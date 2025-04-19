################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/modbus/db.c \
../Core/Src/modbus/modbus.c \
../Core/Src/modbus/modbusMaster.c 

OBJS += \
./Core/Src/modbus/db.o \
./Core/Src/modbus/modbus.o \
./Core/Src/modbus/modbusMaster.o 

C_DEPS += \
./Core/Src/modbus/db.d \
./Core/Src/modbus/modbus.d \
./Core/Src/modbus/modbusMaster.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/modbus/%.o Core/Src/modbus/%.su Core/Src/modbus/%.cyclo: ../Core/Src/modbus/%.c Core/Src/modbus/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-modbus

clean-Core-2f-Src-2f-modbus:
	-$(RM) ./Core/Src/modbus/db.cyclo ./Core/Src/modbus/db.d ./Core/Src/modbus/db.o ./Core/Src/modbus/db.su ./Core/Src/modbus/modbus.cyclo ./Core/Src/modbus/modbus.d ./Core/Src/modbus/modbus.o ./Core/Src/modbus/modbus.su ./Core/Src/modbus/modbusMaster.cyclo ./Core/Src/modbus/modbusMaster.d ./Core/Src/modbus/modbusMaster.o ./Core/Src/modbus/modbusMaster.su

.PHONY: clean-Core-2f-Src-2f-modbus

