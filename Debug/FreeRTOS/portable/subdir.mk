################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS/portable/heap_4.c \
../FreeRTOS/portable/port.c 

OBJS += \
./FreeRTOS/portable/heap_4.o \
./FreeRTOS/portable/port.o 

C_DEPS += \
./FreeRTOS/portable/heap_4.d \
./FreeRTOS/portable/port.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/portable/%.o FreeRTOS/portable/%.su FreeRTOS/portable/%.cyclo: ../FreeRTOS/portable/%.c FreeRTOS/portable/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F401RE -DSTM32 -DSTM32F401RETx -DSTM32F4 -c -I../Inc -I"C:/Users/Windows/Documents/STM32_Projects/freeRTOS-dht22/FreeRTOS/portable" -I"C:/Users/Windows/Documents/STM32_Projects/freeRTOS-dht22/FreeRTOS/include" -I"C:/Users/Windows/Documents/STM32_Projects/freeRTOS-dht22/FreeRTOS" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-FreeRTOS-2f-portable

clean-FreeRTOS-2f-portable:
	-$(RM) ./FreeRTOS/portable/heap_4.cyclo ./FreeRTOS/portable/heap_4.d ./FreeRTOS/portable/heap_4.o ./FreeRTOS/portable/heap_4.su ./FreeRTOS/portable/port.cyclo ./FreeRTOS/portable/port.d ./FreeRTOS/portable/port.o ./FreeRTOS/portable/port.su

.PHONY: clean-FreeRTOS-2f-portable

