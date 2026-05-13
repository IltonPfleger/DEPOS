MAKEFLAGS += --warn-undefined-variables

HERE := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

INCLUDE      := $(HERE)/include
BUILD        := $(HERE)/build
APPLICATIONS := $(HERE)/app
TOOLS        := $(HERE)/tools

SYSTEM      := $(BUILD)/DEPOS
IMAGE       := $(BUILD)/Image.bin
CONFIG      := $(BUILD)/Config

CONFIGURATOR := $(TOOLS)/TraitsLoggerGenerator
TRAITS := $(shell find $(HERE) -name "Traits.hpp")
MAPPER   := $(BUILD)/Mapper

TOOL    := riscv64-linux-gnu
CC      := $(TOOL)-g++
LD      := $(TOOL)-ld
NM      := $(TOOL)-nm
SIZE    := $(TOOL)-size
OBJCOPY := $(TOOL)-objcopy
GDB     := $(TOOL)-gdb
DD      := dd
TRUNCATE := truncate
QEMU    := qemu-system-riscv64

MACHINE ?= virt
APPLICATION ?= HelloWorld

CCFLAGS = -std=c++23 -I$(HERE) -I$(INCLUDE) -Wall -Wextra -Werror -pedantic
CCFLAGS += -D__MACHINE=$(MACHINE) -D__APPLICATION=$(APPLICATION) -g -O3

ifeq ($(MAKELEVEL),0)
run norun debug: $(CONFIG)
	$(MAKE) $@
else
build: $(IMAGE)
endif

$(CONFIG): $(TRAITS)
	mkdir -p $(dir $@)
	g++ $(CCFLAGS) -E $(HERE)/include/Traits.hpp -w -Wno-error=pragma-once-outside-header | $(CONFIGURATOR) > $(CONFIG).cpp
	g++ $(CCFLAGS) $(CONFIG).cpp -o $(CONFIG).elf
	$(CONFIG).elf > $@

MARCH_CCFLAGS = $(CCFLAGS)
ifneq ($(MAKECMDGOALS),clean)
-include $(CONFIG)
include $(HERE)/include/machine/$(MACHINE)/Makedefs.mk
endif
