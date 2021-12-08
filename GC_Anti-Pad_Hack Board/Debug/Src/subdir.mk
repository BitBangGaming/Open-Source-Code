################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/gc_controller_emulation.c \
../Src/main.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_stm32f4xx.c 

OBJS += \
./Src/gc_controller_emulation.o \
./Src/main.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_stm32f4xx.o 

C_DEPS += \
./Src/gc_controller_emulation.d \
./Src/main.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Src/gc_controller_emulation.o: ../Src/gc_controller_emulation.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Inc -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/STM32F4xx_HAL_Driver/Inc" -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/gc_controller_emulation.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/main.o: ../Src/main.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Inc -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/STM32F4xx_HAL_Driver/Inc" -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/main.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/syscalls.o: ../Src/syscalls.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Inc -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/STM32F4xx_HAL_Driver/Inc" -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/syscalls.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/sysmem.o: ../Src/sysmem.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Inc -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/STM32F4xx_HAL_Driver/Inc" -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/sysmem.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/system_stm32f4xx.o: ../Src/system_stm32f4xx.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Inc -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/jcorleto/Documents/Altium Docs/Open-Source-Code/GC_Anti-Pad_Hack Board/Drivers/STM32F4xx_HAL_Driver/Inc" -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/system_stm32f4xx.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

