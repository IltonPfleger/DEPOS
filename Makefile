include Makedefs.mk

TOOLS_OBJECTS := $(patsubst $(TOOLS)/%.cpp,$(BUILD)/%, $(wildcard $(TOOLS)/*.cpp))
SOURCES       := $(shell find src -name '*.cpp')
OBJECTS       := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SOURCES))
DEPENDENCIES  := $(OBJECTS:.o=.d)
MAP           := $(BUILD)/MemoryMap

APPLICATION_ELF       := $(BUILD)/$(APPLICATION).elf
APPLICATION_BINARY       := $(BUILD)/$(APPLICATION).bin

norun: $(IMAGE)

run: $(IMAGE)
	-$(QEMU) -M $(MACHINE_NAME) -smp $(CPUS) -bios none -nographic -m $(MEMORY_SIZE)b -kernel $(IMAGE)

debug: $(IMAGE)
	-$(QEMU) -M $(MACHINE_NAME) -smp $(CPUS) -bios none -nographic -m $(MEMORY_SIZE)b -kernel $(IMAGE) -S -gdb tcp::1234

gdb:
	riscv64-linux-gnu-gdb -ex "file build/DEPOS.elf" -ex "target extended-remote:1234"

#		#-ex "set confirm off"\
	#		#-ex "add-inferior"\
	#		#-ex "inferior 2"\
	#		#-ex "attach 2"\
	#		#-ex "set confirm off"
#		#-ex "file $(SYSTEM).elf"


$(IMAGE): $(SYSTEM).bin $(APPLICATION_BINARY)
	$(DD) bs=1M conv=notrunc if=$(SYSTEM).bin of=$(IMAGE)
	$(DD) bs=1M conv=notrunc if=$(APPLICATION_BINARY) of=$(IMAGE) oflag=seek_bytes seek=$$(( $(APPLICATION_ADDR) - $(RAM_START) ))
	$(TRUNCATE) -s %$(PAGE_SIZE) $(IMAGE)

$(SYSTEM).bin : $(SYSTEM).elf $(TOOLS_OBJECTS) $(APPLICATION_ELF)
	$(MAPPER) $(APPLICATION_ELF) $(MAP)
	$(OBJCOPY) --update-section .__app_mm__=$(MAP) $(SYSTEM).elf
	$(MAPPER) $(SYSTEM).elf $(MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MAP) $(SYSTEM).elf
	$(OBJCOPY) -O binary $(SYSTEM).elf $(SYSTEM).bin

$(APPLICATION_BINARY): $(APPLICATION_ELF)
	$(OBJCOPY) -O binary $< $@

$(APPLICATION_ELF): $(SYSTEM).elf
	make APPLICATION=$(APPLICATION) -C $(APPLICATIONS) all

$(SYSTEM).elf: $(OBJECTS)
	$(LD) --no-gc-sections -e _init --section-start=.init=$(KERNEL_ADDR) --image-base=$(KERNEL_ADDR) -o $@ $(OBJECTS)

$(BUILD)/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(MARCH_CCFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/%: tools/%.cpp 
	mkdir -p $(dir $@)
	g++ $(CCFLAGS) -o $@ $<

clean:
	rm -rf $(APPLICATIONS)/build
	rm -rf build

-include $(DEPENDENCIES)
