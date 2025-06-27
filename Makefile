TOOL := riscv64-unknown-elf
CC := $(TOOL)-g++
AS := $(TOOL)-as
LD := $(TOOL)-ld
OBJCOPY := $(TOOL)-objcopy
QEMU := qemu-system-riscv64


CFLAGS := -Wall -Wextra -pedantic -Iinclude -c -mcmodel=medany
CFLAGS += -ffreestanding -fno-exceptions -fno-rtti -nostdlib  -nostartfiles
CFLAGS += -g -std=c++23 -fmodules-ts 

BUILD := build
TARGET := $(BUILD)/quark
OBJS := $(shell find ./src -type f -name "*.cpp" | sed 's|\./|\./build/|g' | sed 's|\.cpp|.o|')
MODULES_OBJS := $(shell find modules/ -type f -name "*.cpp" | sed 's|^modules/|build/modules/|' | sed 's|\.cpp|.o|')
MODULES_DEPS := $(shell find modules -type f -name "*.cpp" | sed 's|^|build/|g' | sed 's|\.cpp|\.d|g')

all:
	make run

run: $(TARGET)
	$(QEMU) -machine virt -bios $(TARGET) -nographic -m 1024

debug: $(TARGET)
	pkill -f 'qemu.*'
	$(QEMU) -machine virt -bios $(TARGET) -nographic -m 1024 -smp 1 -gdb tcp::1234 -S &
	sleep 1
	alacritty -e sh -c 'gdb -ex "target remote tcp::1234" -ex "file build/quark.elf"'

$(TARGET): $(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJS) $(MODULES_OBJS)
	$(LD) -T linker.ld $^ -o $@

$(BUILD)/src/%.o: src/%.cpp $(MODULES_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/%.o: modules/%.cpp $(MODULES_DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/%.d: modules/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) > $@

-include $(MODULES_DEPS)

clean:
	rm -rf build
	rm -rf gcm.cache
