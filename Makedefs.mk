HERE := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

INCLUDE      := $(HERE)/include
BUILD        := $(HERE)/build
APPLICATIONS := $(HERE)/app
TOOLS        := $(HERE)/tools
SYSTEM       := $(BUILD)/DEPOS

TRAITS := $(TOOLS)/Traits
ELFMAP   := $(BUILD)/ElfMap

TOOL    := riscv64-linux-gnu
CC      := $(TOOL)-g++
LD      := $(TOOL)-ld
NM      := $(TOOL)-nm
SIZE    := $(TOOL)-size
OBJCOPY := $(TOOL)-objcopy
DD      := dd
TRUNCATE := truncate
QEMU    := qemu-system-riscv64

MachineName     := $(shell $(TRAITS) $(INCLUDE) Traits.hpp "Traits<Machine>::NAME")
CPUS            := $(shell $(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<CPUS>::COUNT")
MemorySize      := $(shell $(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<Memory>::Size")
PageSize        := $(shell $(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<Memory>::PageSize")
RamStart        := $(shell printf "0x%x\n" $$($(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<MemoryMap>::RamStart"))
SystemAddr        := $(shell printf "0x%x\n" $$($(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<MemoryMap>::SystemAddr"))
ApplicationAddr := $(shell printf "0x%x\n" $$($(TRAITS) $(INCLUDE) machine/$(MachineName)/Traits.hpp "Traits<MemoryMap>::ApplicationAddr"))
Hypervisor     := $(shell $(TRAITS) $(INCLUDE) Traits.hpp "Traits<System>::Hypervisor")

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -D__MACHINE=$(MachineName)
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
CFLAGS += -msmall-data-limit=0
CFLAGS += -march=rv64ima_zicsr -mabi=lp64
CFLAGS += -g -std=c++2c
