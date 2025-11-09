include Makedefs.mk

TARGET := $(KERNEL)
TOOLS := $(patsubst tools/%.cpp,$(BUILD)/%,$(shell find tools -type f -name "*.cpp"))
SRCS := $(shell find src -type f -name "*.cpp")
OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
LINKER := $(BUILD)/linker.ld

CPUS=$(shell ./Meta get $(TRAITS) Traits::Machine::CPUS)
MACHINE=$(shell ./Meta get $(TRAITS) Traits::Machine::NAME)
MEMORY=$(shell ./Meta get $(TRAITS) Traits::Memory::SIZE)
BOOT_ADDR=$(shell ./Meta get $(TRAITS) Traits::System::ADDR)
KERNEL_END ?=

run: $(TOOLS) $(TARGET).elf
	(cd app && make APPLICATION=$(APPLICATION) KERNEL_ADDR=$(BOOT_ADDR))
	$(MEMORY_MAP_GENERATOR) $(MEMORY_MAP)
	$(OBJCOPY) --update-section .MemoryMap=$(MEMORY_MAP) $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET)
	#KERNEL_END=$$($(NM) $(TARGET).elf | grep __KERNEL_END__ | awk '{print $$1}'); \
	#echo "Kernel end address: $$KERNEL_END"; \
	#$(LD) -Ttext=$$KERNEL_END -o build/$(APPLICATION) build/$(APPLICATION).elf --just-symbols $(TARGET).elf
	#@echo "Kernel end address: $$KERNEL_END"
	$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios none -nographic -m $(MEMORY)b -device loader,file=$(TARGET),addr=$(BOOT_ADDR),force-raw=on

#@( \
	#	TMP=$$(mktemp); \
	#	cat $(TARGET) app/build/$(APPLICATION).elf > $$TMP; \
	#	$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios none -nographic -m 1024 -device loader,file=$$TMP,addr=$(BOOT_ADDR),force-raw=on\
	#)
	

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

$(BUILD)/MemoryMapGenerator: $(TARGET).elf tools/MemoryMapGenerator.cpp 
	KERNEL_END=$$($(NM) $(TARGET).elf | grep __KERNEL_END__ | awk '{print $$1}') && \
	g++ -D__KERNEL_END__=0x$$KERNEL_END -I$(INCLUDE) tools/MemoryMapGenerator.cpp -o build/MemoryMapGenerator


$(TARGET).elf: $(OBJS)
	touch $(LINKER)
	./Meta linker $(LINKER) $(BOOT_ADDR)
	$(LD) -T $(LINKER) -o $@ $(OBJS)

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -MMD -MP -c $< -o $@

clean:
	rm -rf build
	(cd app && make clean)

-include $(DEPS)
