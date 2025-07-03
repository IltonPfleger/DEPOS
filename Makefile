TOOL := riscv64-unknown-elf
CC := $(TOOL)-g++
AS := $(TOOL)-as
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64


CFLAGS := -Wall -Wextra -pedantic -mcmodel=medany -Iinclude
CFLAGS += -ffreestanding -fno-exceptions -fno-rtti -nostdlib  -nostartfiles -fcheck-new
CFLAGS += -g -std=c++23 -march=rv64imac_zicsr -mabi=lp64 -O2

BUILD := build
TARGET := $(BUILD)/quark
OBJS := $(shell find . -type f -name "*.cpp" | sed 's|\./|\./build/|g' | sed 's|\.cpp|.o|')
DEPS = $(OBJS:.o=.d)

all:
	make run

run: $(TARGET)
	$(QEMU) -M sifive_u -bios $(TARGET) -nographic -m 1024 

debug: $(TARGET)
	$(QEMU) -M sifive_u -bios $(TARGET) -nographic -m 1024 -gdb tcp::1234 -S

$(TARGET): $(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJS)
	$(LD) -T linker.ld $^ -o $@

$(BUILD)/%.o: %.cpp 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf build
	rm -rf gcm.cache
