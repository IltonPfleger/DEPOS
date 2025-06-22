TOOL := riscv64-unknown-elf
CC := $(TOOL)-gcc
AS := $(TOOL)-as
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64


CFLAGS := -O0 -Wall -Wextra -pedantic -Iinclude -c -mcmodel=medany
CFLAGS += -ffreestanding -fno-exceptions -fno-rtti -nostdlib  -nostartfiles
CFLAGS += -g -fcheck-new

BUILD := build
TARGET := $(BUILD)/quark
OBJ := $(shell find . -type f -name "*.cpp" | sed -e 's|^\./|$(BUILD)/|' -e 's|\.cpp|\.o|')

all: 
	make clean
	make build 

build: $(TARGET)
	$(QEMU) -machine virt -bios $(TARGET) -nographic -m 1024

debug: $(TARGET)
	$(QEMU) -machine virt -bios $(TARGET) -nographic -m 1024 -smp 1 -gdb tcp::1234 -S
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


