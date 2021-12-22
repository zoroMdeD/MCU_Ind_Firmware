################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/command.c \
../Core/Src/fonts.c \
../Core/Src/gpio.c \
../Core/Src/main.c \
../Core/Src/menu.c \
../Core/Src/rtc.c \
../Core/Src/spi.c \
../Core/Src/st7735.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/tim.c 

OBJS += \
./Core/Src/adc.o \
./Core/Src/command.o \
./Core/Src/fonts.o \
./Core/Src/gpio.o \
./Core/Src/main.o \
./Core/Src/menu.o \
./Core/Src/rtc.o \
./Core/Src/spi.o \
./Core/Src/st7735.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/tim.o 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/command.d \
./Core/Src/fonts.d \
./Core/Src/gpio.d \
./Core/Src/main.d \
./Core/Src/menu.d \
./Core/Src/rtc.d \
./Core/Src/spi.d \
./Core/Src/st7735.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/tim.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/command.d ./Core/Src/command.o ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/menu.d ./Core/Src/menu.o ./Core/Src/rtc.d ./Core/Src/rtc.o ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/st7735.d ./Core/Src/st7735.o ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/tim.d ./Core/Src/tim.o

.PHONY: clean-Core-2f-Src

