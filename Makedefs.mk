INCLUDE := include
BUILD := build
TOOLS := tools
APPLICATIONS := app
TRAITS := include/Traits.hpp
KERNEL := $(BUILD)/DEPOS
IMAGE := $(BUILD)/Image

TOOL := riscv64-linux-gnu
CC := $(TOOL)-g++
LD := $(TOOL)-ld
NM := $(TOOL)-nm
SIZE := $(TOOL)-size
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64
TRUNCATE := truncate
DD := dd

GET := ./Meta get
EPRINT := $(TOOLS)/EPrint
EMAP := $(BUILD)/EMap

CPUS=$(shell $(GET) "Traits<Machine>::CPUS")
MACHINE=$(shell $(GET) "Traits<Machine>::NAME")
MEMORY_SIZE=$(shell $(GET) "Traits<Memory>::SIZE")
PhysicalRamStart=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::PhysicalRamStart"))
PhysicalBootAddr=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::PhysicalBootAddr"))
SystemAddr=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::SystemAddr"))
ApplicationAddr=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::ApplicationAddr"))
PhysicalApplicationAddr=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::PhysicalApplicationAddr"))
PAGE_SIZE=$(shell $(GET) "Traits<Memory>::PAGE_SIZE")
MULTITASK=$(shell $(GET) "Traits<System>::MULTITASK")

CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
CFLAGS += -msmall-data-limit=0
#CFLAGS += -fno-unwind-tables -fno-asynchronous-unwind-tables 
#CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -march=rv64ima_zicsr -mabi=lp64
CFLAGS += -std=c++2c -Oz

#LDFLAGS = --gc-sections --strip-all --discard-all

#ifeq ($(MULTITASK), 1)
#	LDFLAGS += --section-alignment=$(PAGE_SIZE)
#endif




