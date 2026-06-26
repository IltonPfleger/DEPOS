include Makedefs.mk

KERNEL_SOURCES       := $(shell find src -name '*.cpp' | grep -v '*architecture*' | grep -v '*machine*')
KERNEL_SOURCES 	     += $(shell find src/architecture/$(ARCH) -name '*.cpp')
KERNEL_OBJECTS       := $(patsubst src/%.cpp,$(BUILD)/%.o,$(KERNEL_SOURCES))
KERNEL_DEPENDENCIES  := $(KERNEL_OBJECTS:.o=.d)
PAYLOAD_ELF                  := $(BUILD)/$(PAYLOAD).elf
PAYLOAD_BINARY               := $(BUILD)/$(PAYLOAD).bin
MAP                  := $(BUILD)/MemoryMap

run: $(IMAGE).img
	-$(QEMU) -M $(MACHINE) -smp $(CPU_Count) -bios none -nographic -m $(Memory_Size)b -kernel $<

debug: $(IMAGE).img
	-$(QEMU) -M $(MACHINE) -smp $(CPU_Count) -bios none -nographic -m $(Memory_Size)b -kernel $< -S -gdb tcp::1234

gdb:
	$(GDB) -ex "file build/QUARK.elf" -ex "target extended-remote:1234"\

$(IMAGE).bin: $(SYSTEM_BINARY) $(PAYLOAD_BINARY)
	$(DD) bs=1M conv=notrunc if=$(SYSTEM_BINARY) of=$@
	$(DD) bs=1M conv=notrunc if=$(PAYLOAD_BINARY) of=$@ oflag=seek_bytes seek=$$(( $(Payload_Address) - $(MemoryMap_BootStart) ))

$(SYSTEM_BINARY) : $(SYSTEM_ELF) $(PAYLOAD_ELF) $(MAPPER)
	$(MAPPER) $(PAYLOAD_ELF) $(MAP)
	$(OBJCOPY) --update-section .__payload_mm__=$(MAP) $(SYSTEM_ELF)
	$(MAPPER) $(SYSTEM_ELF) $(MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MAP) $(SYSTEM_ELF)
	$(OBJCOPY) -O binary $(SYSTEM_ELF) $(SYSTEM_BINARY)

$(PAYLOAD_ELF): $(SYSTEM_ELF)
	make PAYLOAD=$(PAYLOAD) -C $(PAYLOADS) all

$(SYSTEM_ELF): $(KERNEL_OBJECTS)
	$(LD) --no-gc-sections -e _init --section-start=.init=$(MemoryMap_BootStart) --image-base=$(MemoryMap_BootStart) -o $@ $^

$(BUILD)/%: $(TOOLS)/%.cpp 
	mkdir -p $(dir $@)
	g++ $(CCFLAGS) -o $@ $<

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(MACH_CCFLAGS) -MMD -MP -c $< -o $@

%.bin: %.elf 
	$(OBJCOPY) -O binary $< $@

$(KERNEL_OBJECTS): $(CONFIG)

clean:
	rm -rf build

-include $(KERNEL_DEPENDENCIES)
