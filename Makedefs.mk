TOOL := riscv64-linux-gnu
CC := $(TOOL)-g++
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -fno-exceptions -fno-rtti -ffreestanding -nostdlib -nostartfiles
CFLAGS += -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -g -std=c++2c -g -Os

TRAITS := include/Traits.hpp
