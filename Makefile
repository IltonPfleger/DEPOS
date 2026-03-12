include Makedefs.mk

COMPILED_TOOLS := $(patsubst $(TOOLS)/%.cpp,$(BUILD)/%, $(wildcard $(TOOLS)/*.cpp))
SOURCES       := $(shell find src -name '*.cpp')
OBJECTS       := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SOURCES))
DEPENDENCIES  := $(OBJECTS:.o=.d)
MAP           := $(BUILD)/MemoryMap

ELF       := $(BUILD)/$(APPLICATION).elf
BINARY       := $(BUILD)/$(APPLICATION).bin

ifneq ($(MAKELEVEL),0)

norun: $(IMAGE)

run: $(IMAGE)
	-$(QEMU) -M $(MACHINE_NAME) -smp $(CPUS) -bios none -nographic -m $(MEMORY_SIZE)b -kernel $(IMAGE)

debug: $(IMAGE)
	-$(QEMU) -M $(MACHINE_NAME) -smp $(CPUS) -bios none -nographic -m $(MEMORY_SIZE)b -kernel $(IMAGE) -S -gdb tcp::1234

endif



#gdb:
#	riscv64-linux-gnu-gdb -ex "file build/DEPOS.elf"\
#		-ex "file ../linux/vmlinux"\
#		-ex "target extended-remote:1234"

#		#-ex "set confirm off"\
	#		#-ex "add-inferior"\
	#		#-ex "inferior 2"\
	#		#-ex "attach 2"\
	#		#-ex "set confirm off"
#		#-ex "file $(SYSTEM).elf"
#
$(IMAGE): $(SYSTEM).bin $(BINARY)
	$(DD) bs=1M conv=notrunc if=$(SYSTEM).bin of=$(IMAGE)
	$(DD) bs=1M conv=notrunc if=$(BINARY) of=$(IMAGE) oflag=seek_bytes seek=$$(( $(APPLICATION_ADDR) - $(RAM_START) ))
	$(TRUNCATE) -s %$(PAGE_SIZE) $(IMAGE)

$(SYSTEM).bin : $(SYSTEM).elf $(ELF) $(COMPILED_TOOLS)
	$(MAPPER) $(ELF) $(MAP)
	$(OBJCOPY) --update-section .__app_mm__=$(MAP) $(SYSTEM).elf
	$(MAPPER) $(SYSTEM).elf $(MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MAP) $(SYSTEM).elf
	$(OBJCOPY) -O binary $(SYSTEM).elf $(SYSTEM).bin

$(BINARY): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(ELF): $(SYSTEM).elf
	make APPLICATION=$(APPLICATION) -C $(APPLICATIONS) all

$(SYSTEM).elf: $(OBJECTS)
	$(LD) --no-gc-sections -e _init --section-start=.init=$(KERNEL_ADDR) --image-base=$(KERNEL_ADDR) -o $@ $(OBJECTS)

$(BUILD)/%: $(TOOLS)/%.cpp 
	mkdir -p $(dir $@)
	g++ $(CCFLAGS) -o $@ $<

$(BUILD)/%.o: src/%.cpp $(CONFIG)
	mkdir -p $(dir $@)
	$(CC) $(MARCH_CCFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(APPLICATIONS)/build
	rm -rf build

-include $(DEPENDENCIES)
