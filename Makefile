TOOL := riscv64-unknown-elf
CC := $(TOOL)-gcc
AS := $(TOOL)-as
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64


CFLAGS := -Wall -Wextra -pedantic -Iinclude -c -mcmodel=medany -nostartfiles -nostdlib

BUILD := build
TARGET := $(BUILD)/quark
OBJ := $(shell find . -type f -name "*.c" | sed -e 's|^\./|$(BUILD)/|' -e 's|\.c|\.o|')
DEP = $(OBJ:.o=.d)

build: $(TARGET)
	$(QEMU) -machine virt -bios $(TARGET) -nographic -m 2G -smp 4
	make clean

$(TARGET): $(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJ)
	$(LD) -T linker.ld $^ -o $@

$(BUILD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*


