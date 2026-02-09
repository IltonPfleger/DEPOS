include Makedefs.mk

TOOLS_OBJECTS := $(patsubst $(TOOLS)/%.cpp,$(BUILD)/%, $(wildcard $(TOOLS)/*.cpp))
SOURCES       := $(shell find src -name '*.cpp')
OBJECTS       := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SOURCES))
DEPENDENCIES  := $(OBJECTS:.o=.d)
MAP           := $(BUILD)/MemoryMap

PAYLOAD_INFO       := $(BUILD)/$(APPLICATION).info
PAYLOAD_BIN       := $(BUILD)/$(APPLICATION).bin

norun: $(IMAGE)

run: $(IMAGE)
	-$(QEMU) -M $(MACHINE_NAME) -smp $(CPUS) -bios none -nographic -m $(MEMORY_SIZE)b -kernel $(IMAGE)

debug: $(IMAGE)
	-$(QEMU) -M $(MACHINE_NAME) -smp $(CPUS) -bios none -nographic -m $(MEMORY_SIZE)b -kernel $(IMAGE) -S -gdb tcp::1234

gdb:
	riscv64-linux-gnu-gdb -ex "file build/DEPOS.elf"\
		-ex "file ../linux/vmlinux"\
		-ex "target extended-remote:1234"

#		#-ex "set confirm off"\
	#		#-ex "add-inferior"\
	#		#-ex "inferior 2"\
	#		#-ex "attach 2"\
	#		#-ex "set confirm off"
#		#-ex "file $(SYSTEM).elf"
#
$(IMAGE): $(SYSTEM).bin $(PAYLOAD)
	$(DD) bs=1M conv=notrunc if=$(SYSTEM).bin of=$(IMAGE)
	$(DD) bs=1M conv=notrunc if=$(PAYLOAD_BIN) of=$(IMAGE) oflag=seek_bytes seek=$$(( $(APPLICATION_ADDR) - $(RAM_START) ))
	$(TRUNCATE) -s %$(PAGE_SIZE) $(IMAGE)

$(SYSTEM).bin : $(SYSTEM).elf $(TOOLS_OBJECTS)
	make APPLICATION=$(APPLICATION) -C $(APPLICATIONS) all
	$(MAPPER) $(PAYLOAD_INFO) $(MAP)
	$(OBJCOPY) --update-section .__app_mm__=$(MAP) $(SYSTEM).elf
	$(MAPPER) $(SYSTEM).elf $(MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MAP) $(SYSTEM).elf
	$(OBJCOPY) -O binary $(SYSTEM).elf $(SYSTEM).bin

$(SYSTEM).elf: $(OBJECTS)
	$(LD) -e _init --section-start=.init=$(SYSTEM_ADDR) --image-base=$(SYSTEM_ADDR) -o $@ $(OBJECTS)

$(BUILD)/%: tools/%.cpp 
	mkdir -p $(dir $@)
	g++ $(CCFLAGS) -o $@ $<

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(MARCH_CCFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(APPLICATIONS)/build
	rm -rf build

-include $(DEPENDENCIES)
