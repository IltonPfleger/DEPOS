include Makedefs.mk

TARGET := $(SYSTEM)
APPDIR=$(BUILD)/$(APPLICATION)

TOOLS := $(patsubst tools/%.cpp,$(BUILD)/%,$(shell find tools -type f -name "*.cpp"))
SRCS := $(shell find src -type f -name "*.cpp")
OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
MAP := $(BUILD)/MemoryMap

norun: $(TARGET).bin

run: $(TARGET).bin
	-$(QEMU) -M $(MachineName) -smp $(CPUS) -bios none -nographic -m $(MemorySize)b -kernel $(TARGET).bin

$(TARGET).bin : $(TARGET).elf $(TOOLS)
	make APPLICATION=$(APPLICATION) -C $(APPLICATIONS)
	$(ELFMAP) $(APPDIR).elf $(MAP)
	$(OBJCOPY) --update-section .__app_mm__=$(MAP) $(TARGET).elf
	$(ELFMAP) $(TARGET).elf $(MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MAP) $(TARGET).elf
	$(OBJCOPY) -O binary $(APPDIR).elf $(APPDIR).bin
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin
	$(DD) bs=1 conv=notrunc if=$(APPDIR).bin of=$(TARGET).bin seek=$$(( $(ApplicationAddr) - $(RamStart) ))
	$(TRUNCATE) -s %$(PageSize) $(TARGET).bin

#debug: $(TARGET)
#	$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios none -kernel $(TARGET) -nographic -m 1024 -S -gdb tcp::1234
#
#gdb:
#	riscv64-linux-gnu-gdb\
#		-ex "target extended-remote:1234"\
#		-ex "set confirm off"\
#		-ex "add-inferior"\
#		-ex "inferior 2"\
#		-ex "attach 2"\
#		-ex "set confirm off"\
#		-ex "file $(TARGET).elf"
#

$(TARGET).elf: $(OBJS)
	$(LD) -e _init --section-start=.init=$(BootAddr) --image-base=$(BootAddr) -o $@ $(OBJS)

$(BUILD)/%: tools/%.cpp 
	mkdir -p $(dir $@)
	g++ -Iinclude -o $@ $<

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -MMD -MP -c $< -o $@

clean:
	rm -rf build
	(cd app && make clean)

-include $(DEPS)
