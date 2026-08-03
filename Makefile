# STM32F767 health-check GNU Arm Embedded build.

######################################
# target
######################################
TARGET = stmf7-health-check

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og
# platform
ARCH := $(shell uname -m)
SYS := $(shell uname -s)
# diagnostic output
OUTPUT ?= 1


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

MBEDTLS_DIR = Middlewares/Third_Party/MbedTLS
MBEDTLS_SOURCES = $(filter-out \
$(MBEDTLS_DIR)/library/psa_crypto_storage.c \
$(MBEDTLS_DIR)/library/psa_its_file.c \
$(MBEDTLS_DIR)/library/timing.c, \
$(wildcard $(MBEDTLS_DIR)/library/*.c))

######################################
# source
######################################
# C sources
C_SOURCES = \
Core/Src/common.c \
Core/Src/ethernet_ll.c \
Core/Src/init_ll.c \
Core/Src/main.c \
Core/Src/network_interface.c \
Core/Src/stm32f7xx_it.c \
Core/Src/syscalls.c \
Core/Src/system.c \
Periph/Src/buzzer.c \
Periph/Src/gpio.c \
Periph/Src/spi.c \
Periph/Src/w25q64.c \
Srv/Src/heart_beat.c \
Srv/Src/api_service.c \
Srv/Src/auth_service.c \
Srv/Src/buzzer_service.c \
Srv/Src/health_check_config.c \
Srv/Src/health_check_log.c \
Srv/Src/health_check_service.c \
Srv/Src/network_service.c \
Srv/Src/time_service.c \
Srv/Src/user_store.c \
TLS/Src/tls_platform.c \
TLS/Src/tls_server_credentials.c \
TLS/Src/tls_transport.c \
TLS/Src/tls_trust_store.c \
lwip/system/OS/sys_arch.c \
$(wildcard lwip/core/*.c) \
$(wildcard lwip/core/ipv4/*.c) \
$(wildcard lwip/api/*.c) \
lwip/netif/ethernet.c \
lwip/apps/sntp/sntp.c \
FreeRTOS-Kernel/list.c \
FreeRTOS-Kernel/queue.c \
FreeRTOS-Kernel/tasks.c \
FreeRTOS-Kernel/portable/GCC/ARM_CM7/port.c \
$(MBEDTLS_SOURCES)


# ASM sources
ASM_SOURCES =  \
startup_stm32f767xx.s \
$(wildcard Core/*.s) \
$(wildcard Periph/*.s) \
$(wildcard Srv/*.s)

# ASMM sources
ASMM_SOURCES = \
$(wildcard Core/*.S) \
$(wildcard Periph/*.S) \
$(wildcard Srv/*.S)


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
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
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DHSE_VALUE=8000000 \
-DHSE_STARTUP_TIMEOUT=100 \
-DLSE_STARTUP_TIMEOUT=5000 \
-DLSE_VALUE=32768 \
-DEXTERNAL_CLOCK_VALUE=12288000 \
-DHSI_VALUE=16000000 \
-DLSI_VALUE=32000 \
-DVDD_VALUE=3300 \
-DPREFETCH_ENABLE=1 \
-DINSTRUCTION_CACHE_ENABLE=1 \
-DDATA_CACHE_ENABLE=1 \
-DSTM32F767xx \
-DUSE_FULL_ASSERT \
-DLWIP_IPV4=1 \
-DLWIP_ETHERNET=1 \
-DLWIP_ARP=1 \
-DLWIP_TIMERS=1 \
-DLWIP_NETIF_LINK_CALLBACK=1 \
-DLWIP_ACD=1 \
-DLWIP_DHCP=1 \
-DMBEDTLS_CONFIG_FILE='<health_checker_mbedtls_config.h>'

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-ICore/Inc \
-IPeriph/Inc \
-ISrv/Inc \
-ITLS/Inc \
-ITLS/Private \
-IMiddlewares/Third_Party/MbedTLS/include \
-IDrivers/CMSIS/Device/ST/STM32F7xx/Include \
-IDrivers/CMSIS/Include \
-IDrivers/CMSIS/Include \
-IFreeRTOS-Kernel/include \
-IFreeRTOS-Kernel/portable/GCC/ARM_CM7 \
-Ilwip/system \
-Ilwip/system/arch \
-Ilwip/include \
-Ilwip/include/lwip \
-Ilwip/include/lwip/apps \
-Ilwip/include/lwip/priv \
-Ilwip/include/lwip/prot \
-Ilwip/include/netif \
-Ilwip/include/compat/stdc \
-Ilwip/include/compat/posix \
-Ilwip/include/compat/posix/arpa \
-Ilwip/include/compat/posix/net \
-Ilwip/include/compat/posix/sys



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections


ifeq ($(DEBUG), 1)
# CFLAGS += -g -gdwarf-2 -D CMAKE_CXX_FLAGS_RELEASE="-Wa,-mimplicit-it=thumb"
# CFLAGS += -g -gdwarf-2 -Wextra -pedantic
DEBUGFLAGS = -g -gdwarf-2 -DDEBUG
CFLAGS += $(DEBUGFLAGS)
ASFLAGS += $(DEBUGFLAGS)
endif

ifeq ($(OUTPUT), 1)
OUTPUTFLAGS = -DUSART_OUT=USART3
CFLAGS += $(OUTPUTFLAGS)
ASFLAGS += $(OUTPUTFLAGS)
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F767ZITx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

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
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMM_SOURCES:.S=.O)))
vpath %.S $(sort $(dir $(ASMM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.O: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) -E $< -o $@
	$(AS) -c $(ASFLAGS) $@ -o $@.o
	mv $@.o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@


#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
