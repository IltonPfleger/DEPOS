TOOL := riscv64-linux-gnu
CC := $(TOOL)-g++
LD := $(TOOL)-ld
NM := $(TOOL)-nm
SIZE := $(TOOL)-size
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64
INCLUDE := include

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -fno-exceptions -fno-rtti -ffreestanding -nostdlib -nostartfiles
CFLAGS += -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -g -std=c++2c -g -Os

BUILD := build
TRAITS := include/Traits.hpp
KERNEL := $(BUILD)/DEPOS
