HERE := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

INCLUDE      := $(HERE)/include
BUILD        := $(HERE)/build
APPLICATIONS := $(HERE)/app
TOOLS        := $(HERE)/tools

SYSTEM       := $(BUILD)/DEPOS
IMAGE       := $(BUILD)/Image

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

CCFLAGS = -std=c++2c -I$(HERE) -I$(INCLUDE) -D__MACHINE=$(MACHINE) -D__APPLICATION=$(APPLICATION) -Wall -Wextra -Werror -pedantic

$(BUILD):
	mkdir -p $(BUILD)

$(TRAITS).mk: $(TRAITS)
	$< > $@

$(TRAITS): tools/Traits.cpp $(BUILD)
	g++ $(CCFLAGS) $< -o $@

-include $(TRAITS).mk

#MachineName     := $(shell $(TRAITS) $(INCLUDE) Traits.hpp "Traits<Machine>::NAME")
#CPUS            := $(shell $(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<CPUS>::COUNT")
#MemorySize      := $(shell $(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<Memory>::Size")
#PageSize        := $(shell $(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<Memory>::PageSize")
#RamStart        := $(shell printf "0x%x\n" $$($(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<MemoryMap>::RamStart"))
#SystemAddr        := $(shell printf "0x%x\n" $$($(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<MemoryMap>::SystemAddr"))
#ApplicationAddr := $(shell printf "0x%x\n" $$($(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<MemoryMap>::ApplicationAddr"))
#Hypervisor     := $(shell $(TRAITS) $(INCLUDE) Traits.hpp "Traits<System>::Hypervisor")

MARCH_CCFLAGS = $(CCFLAGS)
MARCH_CCFLAGS += -mcmodel=medany
MARCH_CCFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
MARCH_CCFLAGS += -msmall-data-limit=0
MARCH_CCFLAGS += -march=rv64ima_zicsr -mabi=lp64
MARCH_CCFLAGS += -g 
