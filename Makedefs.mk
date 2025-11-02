TOOL := riscv64-linux-gnu
CC := $(TOOL)-g++
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-exceptions -fno-rtti -nostdlib
CFLAGS += -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -g -std=c++2c -g -O3

TRAITS := include/Traits.hpp
