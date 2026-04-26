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
	-$(QEMU) -M $(MACHINE) -smp $(CPU_Count) -bios none -nographic -m $(Memory_Size)b -kernel $(IMAGE)

debug: $(IMAGE)
	-$(QEMU) -M $(MACHINE) -smp $(CPU_Count) -bios none -nographic -m $(Memory_Size)b -kernel $(IMAGE) -S -gdb tcp::1234

endif

$(OBJECTS): $(CONFIG)

gdb:
	$(GDB)\
		-ex "file build/DEPOS.elf"\
		-ex "target extended-remote:1234"\

$(IMAGE): $(SYSTEM).bin $(BINARY)
	$(DD) bs=1M conv=notrunc if=$(SYSTEM).bin of=$(IMAGE)
	$(DD) bs=1M conv=notrunc if=$(BINARY) of=$(IMAGE) oflag=seek_bytes seek=$$(( $(Application_Addr) - $(MemoryMap_BootStart) ))
	$(TRUNCATE) -s %$(Memory_PageSize) $(IMAGE)

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
	$(LD) --no-gc-sections -e _init --section-start=.init=$(MemoryMap_BootStart) --image-base=$(MemoryMap_BootStart) -o $@ $(OBJECTS)

$(BUILD)/%: $(TOOLS)/%.cpp 
	mkdir -p $(dir $@)
	g++ $(CCFLAGS) -o $@ $<

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(MARCH_CCFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf build

-include $(DEPENDENCIES)
