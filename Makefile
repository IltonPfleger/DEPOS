include Makedefs.mk

TARGET := $(SYSTEM)
APPDIR=$(BUILD)/$(APPLICATION)

TOOLS := $(patsubst tools/%.cpp,$(BUILD)/%,$(shell find tools -type f -name "*.cpp"))
SRCS := $(shell find src -type f -name "*.cpp")
OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
MEMORY_MAP := $(BUILD)/MemoryMap

norun: $(TARGET).bin

run: $(TARGET).bin
	-$(QEMU) -M $(MachineName) -smp $(CPUS) -bios none -nographic -m $(MemorySize)b -kernel $(TARGET).bin

$(TARGET).bin : $(TARGET).elf $(APPDIR).elf $(TOOLS)
	$(ELFMAP) $(APPDIR).elf $(MEMORY_MAP)
	$(OBJCOPY) --update-section .__app_mm__=$(MEMORY_MAP) $(TARGET).elf
	$(ELFMAP) $(TARGET).elf $(MEMORY_MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MEMORY_MAP) $(TARGET).elf
	$(OBJCOPY) -O binary $(APPDIR).elf $(APPDIR).bin
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin
	$(DD) bs=1 conv=notrunc if=$(APPDIR).bin of=$(TARGET).bin seek=$$(( $$($(ELFPRINT) $(APPDIR).elf -b) - $(RamStart) ))
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
$(APPDIR).elf: $(TARGET).elf
	make APPLICATION=$(APPLICATION) -C $(APPLICATIONS)

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
