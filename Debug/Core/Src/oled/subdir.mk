################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/oled/ssd1306.c \
../Core/Src/oled/ssd1306_fonts.c \
../Core/Src/oled/ssd1306_tests.c 

OBJS += \
./Core/Src/oled/ssd1306.o \
./Core/Src/oled/ssd1306_fonts.o \
./Core/Src/oled/ssd1306_tests.o 

C_DEPS += \
./Core/Src/oled/ssd1306.d \
./Core/Src/oled/ssd1306_fonts.d \
./Core/Src/oled/ssd1306_tests.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/oled/%.o Core/Src/oled/%.su Core/Src/oled/%.cyclo: ../Core/Src/oled/%.c Core/Src/oled/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-oled

clean-Core-2f-Src-2f-oled:
	-$(RM) ./Core/Src/oled/ssd1306.cyclo ./Core/Src/oled/ssd1306.d ./Core/Src/oled/ssd1306.o ./Core/Src/oled/ssd1306.su ./Core/Src/oled/ssd1306_fonts.cyclo ./Core/Src/oled/ssd1306_fonts.d ./Core/Src/oled/ssd1306_fonts.o ./Core/Src/oled/ssd1306_fonts.su ./Core/Src/oled/ssd1306_tests.cyclo ./Core/Src/oled/ssd1306_tests.d ./Core/Src/oled/ssd1306_tests.o ./Core/Src/oled/ssd1306_tests.su

.PHONY: clean-Core-2f-Src-2f-oled

