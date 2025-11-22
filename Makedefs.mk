INCLUDE := include
BUILD := build
APP := app
TRAITS := include/Traits.hpp
KERNEL := $(BUILD)/DEPOS

TOOL := riscv64-linux-gnu
CC := $(TOOL)-g++
LD := $(TOOL)-ld
NM := $(TOOL)-nm
SIZE := $(TOOL)-size
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64
GET := ./Meta get $(TRAITS)

CPUS=$(shell $(GET) "Traits<Machine>::CPUS")
MACHINE=$(shell $(GET) "Traits<Machine>::NAME")
MEMORY_SIZE=$(shell $(GET) "Traits<Memory>::SIZE")
PhysicalBootAddr=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::PhysicalBootAddr"))
SystemAddr=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::SystemAddr"))
ApplicationAddr=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::ApplicationAddr"))
PAGE_SIZE=$(shell $(GET) "Traits<Memory>::PAGE_SIZE")
MULTITASK=$(shell $(GET) "Traits<System>::MULTITASK")

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
CFLAGS += -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -g -std=c++2c

#LDFLAGS =
#
#ifeq ($(MULTITASK), 1)
#	LDFLAGS += --section-alignment=$(PAGE_SIZE)
#endif




