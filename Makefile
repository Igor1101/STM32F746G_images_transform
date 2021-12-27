DEBUG 				?= 1
LIB_PATH			?= stm32f746g-discovery-gcc

TOOLCHAIN           ?= arm-none-eabi-
CC                  := $(TOOLCHAIN)gcc
CXX 				:= $(TOOLCHAIN)g++
AS                  := $(TOOLCHAIN)as
LD                  := $(TOOLCHAIN)gcc
OBJCOPY             := $(TOOLCHAIN)objcopy
SIZE		    := $(TOOLCHAIN)size
STRIP 		    := $(TOOLCHAIN)strip
# additional
#
ST_INFO		    ?= st-info
ST_FLASH	    ?= st-flash

PRODUCT_NAME        := main
BUILD               := build
PRODUCT_BASE 		:= $(BUILD)/$(PRODUCT_NAME)
PRODUCT_TMP         := $(PRODUCT_BASE).tmp
PRODUCT_ELF			:= $(PRODUCT_BASE).elf
PRODUCT_HEX        	:= $(patsubst %.elf,%.hex,$(PRODUCT_ELF))
PRODUCT_BIN         := $(patsubst %.elf,%.bin,$(PRODUCT_ELF))
DEBUG_SYMBOLS 		:= $(PRODUCT_BASE).debug

# BSP Features; remove any you don't need
#BSP                 := audio camera eeprom lcd qspi sd sdram ts
BSP := 

# Project source
PROJ_SRC            := $(shell find src -name '*.c' -or -name '*.cpp' -or -name '*.s')
PROJ_OBJ            := $(addsuffix .o,$(basename $(PROJ_SRC)))

# CMSIS
CMSIS_DIR           := $(LIB_PATH)/lib/Drivers/CMSIS
CMSIS_DEVICE_DIR    := $(CMSIS_DIR)/Device/ST/STM32F7xx

# Local directory for storing compiled HAL/BSP objects
DEPS                := .deps

# HAL setup
HAL_DIR             := $(LIB_PATH)/lib/Drivers/STM32F7xx_HAL_Driver
HAL_INC             := $(HAL_DIR)/Inc
HAL_SRC             := $(shell find $(HAL_DIR)/Src -name '*.c')
HAL_OBJ_DIR         := $(DEPS)/hal
HAL_OBJ             := $(patsubst %.c,%.o,$(addprefix $(HAL_OBJ_DIR)/, $(notdir $(HAL_SRC))))

# BSP setup
BSP_DIR             := $(LIB_PATH)/lib/Drivers/BSP/STM32746G-Discovery
BSP_INC             := $(BSP_DIR)
BSP_SRC             := $(addsuffix .c, $(addprefix $(BSP_DIR)/stm32746g_discovery_, $(BSP)))
BSP_OBJ_DIR         := $(DEPS)/bsp
BSP_OBJ             := $(patsubst %.c,%.o,$(addprefix $(BSP_OBJ_DIR)/, $(notdir $(BSP_SRC))))

# Include path
INCLUDE := -Iinclude -Iconfig
INCLUDE += -I$(CMSIS_DEVICE_DIR)/Include
INCLUDE += -I$(CMSIS_DIR)/Include
INCLUDE += -I$(HAL_INC)
INCLUDE += -I$(BSP_INC)

MCPU := cortex-m7
PART := STM32F746xx
FLAGS := -mcpu=$(MCPU) -mthumb
CFLAGS := $(FLAGS) -Os -ffunction-sections -fdata-sections $(INCLUDE) -D$(PART) -fmessage-length=0 -fdata-sections -ffunction-sections -Wcast-align -Wcast-qual -Wvla -Wshadow -Wsuggest-attribute=const -Wmissing-format-attribute -Wuninitialized -Winit-self -Wdouble-promotion -Wno-unused-local-typedefs
CXXFLAGS := $(CFLAGS)
LDFLAGS := $(FLAGS) -specs=nano.specs -Wl,--gc-sections   -ffreestanding -Wl,-defsym,__dso_handle=0 -Wl,-Map=build/output.map
ASFLAGS := $(FLAGS)

ifeq ($(DEBUG),1)
	CFLAGS += -g
	LDFLAGS += -g
	ASFLAGS += -g
endif

$(HAL_OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BSP_OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

all: $(PRODUCT_ELF) $(PRODUCT_HEX) $(PRODUCT_BIN) pr_size

$(PRODUCT_ELF): $(PRODUCT_TMP) $(DEBUG_SYMBOLS)
	cp $(PRODUCT_TMP) $(PRODUCT_ELF)
	$(STRIP) --strip-debug --strip-unneeded $(PRODUCT_ELF)
	$(OBJCOPY) --add-gnu-debuglink=$(DEBUG_SYMBOLS) $(PRODUCT_ELF)

$(DEBUG_SYMBOLS): $(PRODUCT_TMP)
	$(OBJCOPY) --only-keep-debug $< $@

$(HAL_OBJ_DIR):
	mkdir -p $(HAL_OBJ_DIR)

$(BSP_OBJ_DIR):
	mkdir -p $(BSP_OBJ_DIR)

$(PRODUCT_TMP): $(BUILD) obj
	$(LD) -T stm32f746.ld $(LDFLAGS) -o $@ $(HAL_OBJ) $(BSP_OBJ) $(PROJ_OBJ)

$(BUILD):
	mkdir -p $(BUILD)

hal_obj: $(HAL_OBJ_DIR) $(HAL_OBJ)

bsp_obj: $(BSP_OBJ_DIR) $(BSP_OBJ)

obj: hal_obj bsp_obj $(PROJ_OBJ)

vpath %.c $(HAL_DIR)/Src $(BSP_DIR)

clean:
	rm -rf .deps $(PROJ_OBJ)

pr_size:
	$(SIZE) $(PRODUCT_ELF)	

st_link_deploy:
	$(ST_INFO) --probe
	$(ST_FLASH) write $(PRODUCT_BIN)  0x08000000
	$(ST_FLASH) reset
.PHONY: hal_obj obj clean all deploy
