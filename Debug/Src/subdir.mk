################################################################################
# Automatically-generated file. Do not edit!
<<<<<<< HEAD
# Toolchain: GNU Tools for STM32 (14.3.rel1)
=======
# Toolchain: GNU Tools for STM32 (13.3.rel1)
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/dht_22.c \
<<<<<<< HEAD
../Src/fpu_init.c \
=======
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
../Src/main.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/dht_22.o \
<<<<<<< HEAD
./Src/fpu_init.o \
=======
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
./Src/main.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/dht_22.d \
<<<<<<< HEAD
./Src/fpu_init.d \
=======
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
./Src/main.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
<<<<<<< HEAD
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F401RE -DSTM32 -DSTM32F401RETx -DSTM32F4 -c -I../Inc -I"C:/Users/Windows/Documents/STM32_Projects/freeRTOS-dht22/FreeRTOS/portable" -I"C:/Users/Windows/Documents/STM32_Projects/freeRTOS-dht22/FreeRTOS/include" -I"C:/Users/Windows/Documents/STM32_Projects/freeRTOS-dht22/FreeRTOS" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
=======
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F401RE -DSTM32 -DSTM32F401RETx -DSTM32F4 -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f

clean: clean-Src

clean-Src:
<<<<<<< HEAD
	-$(RM) ./Src/dht_22.cyclo ./Src/dht_22.d ./Src/dht_22.o ./Src/dht_22.su ./Src/fpu_init.cyclo ./Src/fpu_init.d ./Src/fpu_init.o ./Src/fpu_init.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su
=======
	-$(RM) ./Src/dht_22.cyclo ./Src/dht_22.d ./Src/dht_22.o ./Src/dht_22.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f

.PHONY: clean-Src

