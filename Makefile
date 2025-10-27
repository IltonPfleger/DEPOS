TOOL := riscv64-elf
CC := $(TOOL)-g++
QEMU := qemu-system-riscv64

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -Iinclude
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-exceptions -fno-rtti -nostdlib
CFLAGS += -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -g -std=c++20 -O3 -g



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
	$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios none -kernel $(TARGET) -nographic -m 1024

debug: $(TARGET)
	$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios none -kernel $(TARGET) -nographic -m 1024 -S -gdb tcp::1235

gdb:
	riscv64-linux-gnu-gdb\
		-ex "target extended-remote:1235"\
		-ex "set confirm off"\
		-ex "add-inferior"\
		-ex "inferior 2"\
		-ex "attach 2"\
		-ex "set confirm off"\
		-ex "file $(TARGET).elf"

$(TARGET): $(OBJS)
	@./mkimage $(TOOL) $@ $^

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/$(APPLICATION).o: app/$(APPLICATION).cpp
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf build
