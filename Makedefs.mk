INCLUDE := include
BUILD := build
APPLICATIONS := app
TRAITS := include/Traits.hpp
KERNEL := $(BUILD)/DEPOS

DD := dd
TOOL := riscv64-linux-gnu
CC := $(TOOL)-g++
LD := $(TOOL)-ld
NM := $(TOOL)-nm
SIZE := $(TOOL)-size
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64
GET := ./Meta get $(TRAITS)

CPUS=$(shell $(GET) "Traits<CPUS>::COUNT")
MACHINE=$(shell $(GET) "Traits<Machine>::NAME")
MEMORY_SIZE=$(shell $(GET) "Traits<Memory>::SIZE")
#APP_ADDR=0x$(shell printf "%x\n" $$($(GET) "Traits<Application>::ADDR"))
#PAGE_SIZE=$(shell $(GET) "Traits<Memory>::PAGE_SIZE")
MULTITASK=$(shell $(GET) "Traits<System>::MULTITASK")

RamStart=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::RamStart"))
BootAddr=0x$(shell printf "%x\n" $$($(GET) "Traits<MemoryMap>::BootAddr"))
ApplicationAddr=$(shell printf "0x%x\n" $$($(GET) "Traits<MemoryMap>::ApplicationAddr"))

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
CFLAGS += -march=rv64ima_zicsr -mabi=lp64
CFLAGS += -g -std=c++2c

#LDFLAGS =
#
#ifeq ($(MULTITASK), 1)
#	LDFLAGS += --section-alignment=$(PAGE_SIZE)
#endif




