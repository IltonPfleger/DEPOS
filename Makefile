TOOL := riscv64-linux-gnu
CC := $(TOOL)-g++
LD := $(TOOL)-ld
QEMU := qemu-system-riscv64

CFLAGS = -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -Iinclude
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-exceptions -fno-rtti -nostdlib
CFLAGS += -march=rv64imac_zicsr -mabi=lp64
CFLAGS += -g -std=c++2c -g -O3

BUILD := build
TARGET := $(BUILD)/DEPOS
SRCS := $(shell find src -type f -name "*.cpp")
OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
APPSRCS := $(shell find app/$(APPLICATION) -type f -name "*.cpp")
APPOBJS := $(patsubst app/$(APPLICATION)/%.cpp,$(BUILD)/app/$(APPLICATION)/%.o,$(APPSRCS))
DEPS := $(OBJS:.o=.d) $(APPOBJS:.o=.d)


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

$(TARGET): $(OBJS) $(BUILD)/$(APPLICATION).o
	@./mkimage $(TOOL) $(BUILD)/$(APPLICATION).o $(TARGET) $(OBJS)

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/$(APPLICATION).o: $(APPOBJS)
	@mkdir -p $(dir $@)
	$(LD) -r -o $@ $^

$(BUILD)/app/$(APPLICATION)/%.o: app/$(APPLICATION)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf build
