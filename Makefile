TOOL := riscv64-unknown-elf
CC := $(TOOL)-g++
AS := $(TOOL)-as
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64


CFLAGS := -Wall -Wextra -pedantic -Iinclude -c -mcmodel=medany -nostartfiles -nostdlib

BUILD := build
TARGET := $(BUILD)/quark
OBJ := $(shell find . -type f -name "*.cc" | sed -e 's|^\./|$(BUILD)/|' -e 's|\.cc|\.o|')
DEP = $(OBJ:.o=.d)

run:
	make clean
	make build
	$(QEMU) -machine virt -bios $(TARGET) -nographic -m 2G -smp 4

debug:
	make clean
	make build
	$(QEMU) -bios $(TARGET) -nographic -m 2G -smp 4 -S

build: $(TARGET)

$(TARGET): $(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJ)
	$(LD) -T linker.ld $^ -o $@

$(BUILD)/%.o: %.cc
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*


