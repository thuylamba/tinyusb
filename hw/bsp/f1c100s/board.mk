DEPS_SUBMODULES += hw/mcu/allwinner

DEFINES += -D__ARM32_ARCH__=5 -D__ARM926EJS__

CFLAGS += \
  -march=armv5te \
  -mtune=arm926ej-s \
  -mfloat-abi=soft \
  -marm \
  -mno-thumb-interwork \
  -Wno-unused-parameter \
  -DCFG_TUSB_MCU=OPT_MCU_F1C100S \
  $(DEFINES)

LD_FILE = hw/mcu/allwinner/f1c100s/f1c100s.ld
LDFLAGS += -nostdlib # -lgcc
MCU_DIR = hw/mcu/allwinner/f1c100s

SRC_C += \
	src/portable/sunxi/dcd_sunxi_musb.c \
	$(MCU_DIR)/machine/sys-uart.c \
	$(MCU_DIR)/machine/exception.c \
	$(MCU_DIR)/machine/sys-clock.c \
	$(MCU_DIR)/machine/sys-copyself.c \
	$(MCU_DIR)/machine/sys-dram.c \
	$(MCU_DIR)/machine/sys-mmu.c \
	$(MCU_DIR)/machine/sys-spi-flash.c \
	$(MCU_DIR)/machine/f1c100s-intc.c \
	$(MCU_DIR)/machine/f1c100s-sdc.c \
	$(MCU_DIR)/machine/f1c100s-gpio.c \
	$(MCU_DIR)/lib/malloc.c \
	$(MCU_DIR)/lib/strlen.c \
	$(MCU_DIR)/lib/dma.c \
	$(MCU_DIR)/lib/printf.c 

SRC_S += \
	$(MCU_DIR)/lib/memcpy.S \
	$(MCU_DIR)/lib/memmove.S \
	$(MCU_DIR)/lib/memset.S	\
  $(MCU_DIR)/machine/start.S

INC += \
	$(TOP)/$(MCU_DIR)/include \
	$(TOP)/$(BOARD_PATH)

# flash target using xfel
flash: flash-xfel

exec: $(BUILD)/$(PROJECT).bin
	xfel ddr 
	xfel write 0x80000000 $<
	xfel exec 0x80000000
