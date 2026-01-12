INCLUDE := include
BUILD := build
APPLICATIONS := app
TOOLS := tools
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
TRAITS := $(TOOLS)/Traits
EPRINT := $(TOOLS)/EPrint
EMAP := $(BUILD)/EMap

MachineName=$(shell $(TRAITS) include/Traits.hpp "Traits<Machine>::NAME")
CPUS=$(shell $(TRAITS) include/machine/$(MachineName)/Traits.hpp "Traits<CPUS>::COUNT")
MemorySize=$(shell $(TRAITS) include/machine/$(MachineName)/Traits.hpp "Traits<Memory>::Size")
PageSize=$(shell $(TRAITS) include/machine/$(MachineName)/Traits.hpp "Traits<Memory>::PageSize")
RamStart=$(shell printf "0x%x\n" $$($(TRAITS) include/machine/$(MachineName)/Traits.hpp "Traits<MemoryMap>::RamStart"))
BootAddr=$(shell printf "0x%x\n" $$($(TRAITS) include/machine/$(MachineName)/Traits.hpp "Traits<MemoryMap>::BootAddr"))
ApplicationAddr=$(shell printf "0x%x\n" $$($(TRAITS) include/machine/$(MachineName)/Traits.hpp "Traits<MemoryMap>::ApplicationAddr"))

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -D__MACHINE=$(MachineName)
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
CFLAGS += -msmall-data-limit=0
CFLAGS += -march=rv64ima_zicsr -mabi=lp64
CFLAGS += -g -std=c++2c
