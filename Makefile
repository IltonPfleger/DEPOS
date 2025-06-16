TOOL := riscv64-unknown-elf
CC := $(TOOL)-gcc
AS := $(TOOL)-as
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64


CFLAGS := -O2 -Wall -Wextra -pedantic -Iinclude -c -mcmodel=medany
CFLAGS += -ffreestanding -fno-exceptions -fno-rtti -nostdlib -nostartfiles

BUILD := build
TARGET := $(BUILD)/quark
OBJ := $(shell find . -type f -name "*.cpp" | sed -e 's|^\./|$(BUILD)/|' -e 's|\.cpp|\.o|')

build: $(TARGET)
	$(QEMU) -machine virt -bios $(TARGET) -nographic -m 1024 -smp 4
	make clean

#debug: $(TARGET)
#	$(QEMU) -machine virt -bios $(TARGET) -nographic -m 1G -smp 4 -S -gdb tcp::1234&
#	riscv64-unknown-elf-gdb kernel.elf -ex "target remote :1234"
#	pkill qemu;


$(TARGET): $(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJ)
	$(LD) -T linker.ld $^ -o $@

$(BUILD)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*


