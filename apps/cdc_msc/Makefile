include ../../tools/top.mk
include ../../tools/f1c100s/tools.mk
include ../make.mk

INC += \
	src \
	$(TOP)/hw \

# Example source
CDC_MSC_SRC += $(wildcard src/*.c)
SRC_C += $(addprefix $(CURRENT_PATH)/, $(CDC_MSC_SRC))

include ../rules.mk
