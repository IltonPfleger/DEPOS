TOOL := riscv64-unknown-elf
CC := $(TOOL)-gcc
AS := $(TOOL)-as
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64


CFLAGS := -Wall -Wextra -pedantic -Iinclude -c -mcmodel=medany -nostartfiles -nostdlib

BUILD := build
TARGET := $(BUILD)/quark
OBJ := $(shell find . -type f -name "*.cpp" | sed -e 's|^\./|$(BUILD)/|' -e 's|\.cpp|\.o|')

all: $(TARGET)
	$(QEMU) -machine virt -bios $(TARGET) -nographic -m 2G -smp 4
	make clean

$(TARGET): $(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJ)
	$(LD) -T linker.ld $^ -o $@

$(BUILD)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*


