TOOL := riscv64-elf
CC := $(TOOL)-g++
AS := $(TOOL)-as
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64

CFLAGS := -Wall -Wextra -Werror -pedantic -mcmodel=medany -Iinclude
CFLAGS +=  -ffunction-sections -fdata-sections -ffreestanding -fno-exceptions -fno-rtti -nostdlib  -nostartfiles -fcheck-new -Wno-dangling-pointer -Wno-sized-deallocation
CFLAGS += -g -std=c++20 -march=rv64gc

BUILD := build
TARGET := $(BUILD)/quark
OBJS := $(shell find src/ -type f -name "*.cpp" | sed 's|src/|\./build/|g' | sed 's|\.cpp|.o|')
OBJS += $(BUILD)/$(APPLICATION).o
DEPS = $(OBJS:.o=.d)


CPUS=$(shell sed -n 's|.*CPUS *= *\([0-9]*\).*|\1|p' include/Traits.hpp)
MACHINE=$(shell sed -n 's|.*NAME *= *"\([^*]*\)".*|\1|p' include/Traits.hpp)

default:
	make run

run: $(TARGET)
	$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios $(TARGET) -nographic -m 1024

debug: $(TARGET)
	$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios $(TARGET) -nographic -m 1024 -gdb tcp::1234 -S

gdb:
	 gdb -ex "target remote tcp::1234" -ex "file build/quark.elf"

$(TARGET): $(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJS)
	$(LD) -T linker.ld $^ -o $@

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/$(APPLICATION).o: app/$(APPLICATION).cpp
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf build
