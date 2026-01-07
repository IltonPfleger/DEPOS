INCLUDE := include
BUILD := build
APPLICATIONS := app
TOOLS := tools
TRAITS := include/Traits.hpp
KERNEL := $(BUILD)/DEPOS

DD := dd
TRUNCATE := truncate
TOOL := riscv64-linux-gnu
CC := $(TOOL)-g++
LD := $(TOOL)-ld
NM := $(TOOL)-nm
SIZE := $(TOOL)-size
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64
GET := $(TOOLS)/Traits $(TRAITS)
EPRINT := $(TOOLS)/EPrint
EMAP := $(BUILD)/EMap

CPUS=$(shell $(GET) "Traits<CPUS>::COUNT")
MACHINE=$(shell $(GET) "Traits<Machine>::NAME")
QEMU_MACHINE := $(shell printf '%s\n' $(MACHINE) | tr A-Z a-z)
MemorySize=$(shell $(GET) "Traits<Memory>::Size")
PageSize=$(shell $(GET) "Traits<Memory>::PageSize")
#APP_ADDR=0x$(shell printf "%x\n" $$($(GET) "Traits<Application>::ADDR"))
#PAGE_SIZE=$(shell $(GET) "Traits<Memory>::PAGE_SIZE")
MULTITASK=$(shell $(GET) "Traits<System>::MULTITASK")

RamStart=$(shell printf "0x%x\n" $$($(GET) "Traits<MemoryMap>::RamStart"))
BootAddr=$(shell printf "0x%x\n" $$($(GET) "Traits<MemoryMap>::BootAddr"))
ApplicationAddr=$(shell printf "0x%x\n" $$($(GET) "Traits<MemoryMap>::ApplicationAddr"))

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -D__MACHINE__=$(MACHINE)
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
CFLAGS += -msmall-data-limit=0
CFLAGS += -march=rv64ima_zicsr -mabi=lp64
CFLAGS += -g -std=c++2c
