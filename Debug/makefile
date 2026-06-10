################################################################################
# Automatically-generated file. Do not edit!
<<<<<<< HEAD
# Toolchain: GNU Tools for STM32 (14.3.rel1)
=======
# Toolchain: GNU Tools for STM32 (13.3.rel1)
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
<<<<<<< HEAD
-include FreeRTOS/portable/subdir.mk
-include FreeRTOS/subdir.mk
=======
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


<<<<<<< HEAD
BUILD_ARTIFACT_NAME := freeRTOS-dht22
=======
BUILD_ARTIFACT_NAME := DHT22_1Wire_Telemetery
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
<<<<<<< HEAD
freeRTOS-dht22.elf \

MAP_FILES += \
freeRTOS-dht22.map \
=======
DHT22_1Wire_Telemetery.elf \

MAP_FILES += \
DHT22_1Wire_Telemetery.map \
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
<<<<<<< HEAD
freeRTOS-dht22.list \
=======
DHT22_1Wire_Telemetery.list \
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f


# All Target
all: main-build

# Main-build Target
<<<<<<< HEAD
main-build: freeRTOS-dht22.elf secondary-outputs

# Tool invocations
freeRTOS-dht22.elf freeRTOS-dht22.map: $(OBJS) $(USER_OBJS) C:\Users\Windows\Documents\STM32_Projects\freeRTOS-dht22\STM32F401RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "freeRTOS-dht22.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\Windows\Documents\STM32_Projects\freeRTOS-dht22\STM32F401RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="freeRTOS-dht22.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
=======
main-build: DHT22_1Wire_Telemetery.elf secondary-outputs

# Tool invocations
DHT22_1Wire_Telemetery.elf DHT22_1Wire_Telemetery.map: $(OBJS) $(USER_OBJS) C:\Users\Windows\STM32CubeIDE\workspace_2.0.0\DHT22_1Wire_Telemetery\STM32F401RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "DHT22_1Wire_Telemetery.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\Windows\STM32CubeIDE\workspace_2.0.0\DHT22_1Wire_Telemetery\STM32F401RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="DHT22_1Wire_Telemetery.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

<<<<<<< HEAD
freeRTOS-dht22.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "freeRTOS-dht22.list"
=======
DHT22_1Wire_Telemetery.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "DHT22_1Wire_Telemetery.list"
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
<<<<<<< HEAD
	-$(RM) default.size.stdout freeRTOS-dht22.elf freeRTOS-dht22.list freeRTOS-dht22.map
=======
	-$(RM) DHT22_1Wire_Telemetery.elf DHT22_1Wire_Telemetery.list DHT22_1Wire_Telemetery.map default.size.stdout
>>>>>>> f003be09006fa0f1b50ddbaa1d9e19f3429e3d9f
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
