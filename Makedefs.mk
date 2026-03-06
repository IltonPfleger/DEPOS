HERE := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

INCLUDE      := $(HERE)/include
BUILD        := $(HERE)/build
APPLICATIONS := $(HERE)/app
TOOLS        := $(HERE)/tools

SYSTEM       := $(BUILD)/DEPOS
IMAGE       := $(BUILD)/Image.bin
APPLICATION_STAMP       := $(BUILD)/.stamp

TRAITS := $(BUILD)/Traits
MAPPER   := $(BUILD)/Mapper

TOOL    := riscv64-linux-gnu
CC      := $(TOOL)-g++
LD      := $(TOOL)-ld
NM      := $(TOOL)-nm
SIZE    := $(TOOL)-size
OBJCOPY := $(TOOL)-objcopy
DD      := dd
TRUNCATE := truncate
QEMU    := qemu-system-riscv64

MACHINE ?= virt
APPLICATION ?= HelloWorld

CCFLAGS = -std=c++23 -I$(HERE) -I$(INCLUDE) -D__MACHINE=$(MACHINE) -D__APPLICATION=$(APPLICATION) -Wall -Wextra -Werror -pedantic -Wfatal-errors

build: $(IMAGE)

OLD_APPLICATION_STAMP := $(shell cat $(APPLICATION_STAMP) 2>/dev/null)

ifneq ($(OLD_APPLICATION_STAMP),$(APPLICATION))
.PHONY: $(APPLICATION_STAMP)
endif

$(APPLICATION_STAMP):
	@mkdir -p $(dir $@)
	@echo "$(APPLICATION)" > $@

$(TRAITS).mk: $(TRAITS) $(APPLICATION_STAMP)
	@mkdir -p $(dir $@)
	$< > $@

$(TRAITS): tools/Traits.cpp
	@mkdir -p $(dir $@)
	g++ $(CCFLAGS) $< -o $@


ifneq ($(MAKECMDGOALS),clean)
-include $(TRAITS).mk
endif

MARCH_CCFLAGS = $(CCFLAGS)
MARCH_CCFLAGS += -D__ARCH=$(ARCH)
MARCH_CCFLAGS += -mcmodel=medany
MARCH_CCFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
MARCH_CCFLAGS += -msmall-data-limit=0 -fno-threadsafe-statics -fno-use-cxa-atexit
MARCH_CCFLAGS += -march=rv64g_zicsr -mabi=lp64
MARCH_CCFLAGS += -g -O3


