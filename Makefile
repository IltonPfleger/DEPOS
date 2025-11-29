include Makedefs.mk

TARGET := $(KERNEL)
_APPLICATION=$(APP)/$(BUILD)/$(APPLICATION)

TOOLS := $(patsubst tools/%.cpp,$(BUILD)/%,$(shell find tools -type f -name "*.cpp"))
SRCS := $(shell find src -type f -name "*.cpp")
OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
MEMORY_MAP := $(BUILD)/MemoryMap

run: $(TARGET).elf $(TOOLS)
	#make APPLICATION=$(APPLICATION) -C app
	#$(LD) -e main --just-symbols $(TARGET).elf --image-base=$(ApplicationAddr) -o $(_APPLICATION).elf $(_APPLICATION).o
	#./$(BUILD)/ELFParser $(_APPLICATION).elf $(MEMORY_MAP)
	#$(OBJCOPY) --update-section .__app_mm__=$(MEMORY_MAP) $(TARGET).elf
	./$(BUILD)/ELFParser $(TARGET).elf $(MEMORY_MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MEMORY_MAP) $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin
	#$(QEMU) -nographic -M $(MACHINE) -smp $(CPUS) -bios none -m $(MEMORY_SIZE)b -kernel $(TARGET).bin
	#$(QEMU) -nographic -M $(MACHINE) -smp $(CPUS) -bios none -m $(MEMORY_SIZE)b -kernel $(TARGET).bin -device loader,file=$(_APPLICATION).elf,addr=$(PhysicalApplicationAddr),force-raw=on

debug: $(TARGET)
	$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios none -kernel $(TARGET) -nographic -m 1024 -S -gdb tcp::1234

gdb:
	riscv64-linux-gnu-gdb\
		-ex "target extended-remote:1234"\
		-ex "set confirm off"\
		-ex "add-inferior"\
		-ex "inferior 2"\
		-ex "attach 2"\
		-ex "set confirm off"\
		-ex "file $(TARGET).elf"
		#-ex "add-symbol-file $(TARGET).elf $(PhysicalBootAddr)"\


$(TARGET).elf: $(OBJS)
	$(LD) $(LDFLAGS) -e _init --section-start=.init=$(SystemAddr) --image-base=$(SystemAddr) -o $@ $(OBJS)

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
