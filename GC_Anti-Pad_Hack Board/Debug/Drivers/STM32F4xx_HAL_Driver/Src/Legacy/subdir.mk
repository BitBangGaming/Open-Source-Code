################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/STM32F4xx_HAL_Driver/Src/Legacy/stm32f4xx_hal_can.c 

OBJS += \
./Drivers/STM32F4xx_HAL_Driver/Src/Legacy/stm32f4xx_hal_can.o 

C_DEPS += \
./Drivers/STM32F4xx_HAL_Driver/Src/Legacy/stm32f4xx_hal_can.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32F4xx_HAL_Driver/Src/Legacy/stm32f4xx_hal_can.o: ../Drivers/STM32F4xx_HAL_Driver/Src/Legacy/stm32f4xx_hal_can.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Inc -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/STM32F4xx_HAL_Driver/Inc" -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Drivers/STM32F4xx_HAL_Driver/Src/Legacy/stm32f4xx_hal_can.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

