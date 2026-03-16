HERE := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

INCLUDE      := $(HERE)/include
BUILD        := $(HERE)/build
APPLICATIONS := $(HERE)/app
TOOLS        := $(HERE)/tools

SYSTEM      := $(BUILD)/DEPOS
IMAGE       := $(BUILD)/Image.bin
CONFIG      := $(BUILD)/Config

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

CCFLAGS = -std=c++23 -I$(HERE) -I$(INCLUDE) -Wall -Wextra -Werror -pedantic -Wfatal-errors
CCFLAGS += -D__MACHINE=$(MACHINE) -D__ARCH=$(ARCH) -D__APPLICATION=$(APPLICATION) 

ifeq ($(MAKELEVEL),0)
run norun debug: $(CONFIG)
	$(MAKE) $@
else
build: $(IMAGE)
endif

.PHONY: $(CONFIG)
$(CONFIG):
	mkdir -p $(dir $@)
	g++ $(CCFLAGS) $(TOOLS)/Traits.cpp -o $(TRAITS)
	$(TRAITS) > $@.tmp
	@cmp -s $@ $@.tmp && rm -f $@.tmp || (mv -f $@.tmp $@)

MARCH_CCFLAGS = $(CCFLAGS)
ifneq ($(MAKECMDGOALS),clean)
-include $(CONFIG)
include $(HERE)/include/machine/$(MACHINE)/Makedefs.mk
endif
