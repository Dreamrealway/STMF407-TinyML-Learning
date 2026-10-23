# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ------------------------------------------------

######################################
# target
######################################
TARGET = STM32F407ZGT6


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
USER/main.c \
USER/stm32f4xx_it.c \
USER/stm32f4xx_hal_msp.c \
USER/system_stm32f4xx.c \
USMART/usmart_config.c \
USMART/usmart_str.c \
USMART/usmart.c \
SYSTEM/delay/delay.c \
SYSTEM/sys/sys.c \
SYSTEM/sys/sys_asm.c \
SYSTEM/usart/usart.c \
HARDWARE/24CXX/24cxx.c \
HARDWARE/IIC/myiic.c \
HARDWARE/KEY/key.c \
HARDWARE/LCD/lcd.c \
HARDWARE/LED/led.c \
HARDWARE/TOUCH/ctiic.c \
HARDWARE/TOUCH/ft5206.c \
HARDWARE/TOUCH/gt9147.c \
HARDWARE/TOUCH/ott2001a.c \
HARDWARE/TOUCH/touch.c \
HARDWARE/SRAM/sram.c \
HARDWARE/TINYML/tinyml.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sram.c \
HALLIB/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fsmc.c

# C++ 源文件
CPP_SOURCES = \
HARDWARE/TINYML/tinyml_infer.cpp \


# ASM sources
ASM_SOURCES =  \
startup_stm32f407zgtx.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
GCC_PATH ?= /home/adminuser/my_work/arm_none_eabi/arm-gnu-toolchain-15.3.rel2-x86_64-arm-none-eabi/bin

ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
CXX = $(GCC_PATH)/$(PREFIX)g++ # C++支持
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT_ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT_ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F407xx


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-IUSER \
-IHALLIB/STM32F4xx_HAL_Driver/Inc \
-IHALLIB/STM32F4xx_HAL_Driver/Inc/Legacy \
-ICORE \
-IUSMART \
-ISYSTEM/delay \
-ISYSTEM/sys \
-ISYSTEM/usart \
-IHARDWARE/24CXX \
-IHARDWARE/IIC \
-IHARDWARE/KEY \
-IHARDWARE/LCD \
-IHARDWARE/LED \
-IHARDWARE/TOUCH \
-IHARDWARE/SRAM \
-IHARDWARE/TINYML \
-I../tflite-micro \
-I../tflite-micro/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include \


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections # -DTF_LITE_STATIC_MEMORY TFLM 推荐的宏 千万不要加

CXXFLAGS += -std=gnu++17 -fno-rtti -fno-exceptions -fno-threadsafe-statics 

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F407ZGTX_FLASH.ld

# libraries
LIBS = -lc -lm 
LIBS += -L./HARDWARE/TINYML/LIB 
LIBS += -ltensorflow-microlite 
LIBS += -lstdc++ 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -specs=nosys.specs -T$(LDSCRIPT) $(LIBDIR) -u _printf_float $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin



#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
# list of C++ program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

# C++ 编译规则
$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir -p $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)


flash: all
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"


help:
	@echo "usage: make all GCC_PATH=<xxx>"
	@echo "       make clean"
	@echo "       make flash"
	@echo "       \
	revise the C_SOURCES C_INCLUDES to make object files\
	"
# *** EOF ***
