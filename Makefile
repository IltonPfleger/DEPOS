include Makedefs.mk

SOURCES       := $(shell find src -name '*.cpp')
OBJECTS       := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SOURCES))
DEPENDENCIES  := $(OBJECTS:.o=.d)
MAP           := $(BUILD)/MemoryMap

ELF       := $(BUILD)/$(APPLICATION).elf
BINARY       := $(BUILD)/$(APPLICATION).bin

run: $(IMAGE).img
	-$(QEMU) -M $(MACHINE) -smp $(CPU_Count) -bios none -nographic -m $(Memory_Size)b -kernel $<

debug: $(IMAGE).img
	-$(QEMU) -M $(MACHINE) -smp $(CPU_Count) -bios none -nographic -m $(Memory_Size)b -kernel $< -S -gdb tcp::1234

$(OBJECTS): $(CONFIG)

gdb:
	$(GDB) -ex "file build/DEPOS.elf" -ex "target extended-remote:1234"\

$(IMAGE).bin: $(SYSTEM).bin $(BINARY)
	$(DD) bs=1M conv=notrunc if=$(SYSTEM).bin of=$@
	$(DD) bs=1M conv=notrunc if=$(BINARY) of=$@ oflag=seek_bytes seek=$$(( $(Application_Addr) - $(MemoryMap_BootStart) ))

$(SYSTEM).bin : $(SYSTEM).elf $(ELF) $(MAPPER)
	$(MAPPER) $(ELF) $(MAP)
	$(OBJCOPY) --update-section .__app_mm__=$(MAP) $(SYSTEM).elf
	$(MAPPER) $(SYSTEM).elf $(MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MAP) $(SYSTEM).elf
	$(OBJCOPY) -O binary $(SYSTEM).elf $(SYSTEM).bin

$(ELF): $(SYSTEM).elf
	make APPLICATION=$(APPLICATION) -C $(APPLICATIONS) all

$(SYSTEM).elf: $(OBJECTS)
	$(LD) --no-gc-sections -e _init --section-start=.init=$(MemoryMap_BootStart) --image-base=$(MemoryMap_BootStart) -o $@ $^

$(BUILD)/%: $(TOOLS)/%.cpp 
	mkdir -p $(dir $@)
	g++ $(CCFLAGS) -o $@ $<

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(MACH_CCFLAGS) -MMD -MP -c $< -o $@

%.bin: %.elf 
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf build

-include $(DEPENDENCIES)
