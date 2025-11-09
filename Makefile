include Makedefs.mk

TARGET := $(KERNEL)
TOOLS := $(patsubst tools/%.cpp,$(BUILD)/%,$(shell find tools -type f -name "*.cpp"))
SRCS := $(shell find src -type f -name "*.cpp")
OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
LINKER := $(BUILD)/linker.ld
SYMBOLS := $(BUILD)/symbols.h
MEMORY_MAP := $(BUILD)/MemoryMap

CPUS=$(shell ./Meta get $(TRAITS) Traits::Machine::CPUS)
MACHINE=$(shell ./Meta get $(TRAITS) Traits::Machine::NAME)
MEMORY=$(shell ./Meta get $(TRAITS) Traits::Memory::SIZE)
BOOT_ADDR=$(shell ./Meta get $(TRAITS) Traits::System::ADDR)
APP_ADDR=$(shell ./Meta get $(TRAITS) Traits::Application::ADDR)

run: $(TARGET).elf
	make APPLICATION=$(APPLICATION) -C app
	./Linker $(APP_ADDR) LD_APPLICATION > $(LINKER)
	$(LD) -T $(LINKER) --just-symbols $(TARGET).elf $(BUILD)/$(APPLICATION).o -o $(BUILD)/$(APPLICATION).elf
	$(NM) -n $(BUILD)/$(APPLICATION).elf | grep -e LD -e main | awk '{if($$3!="") printf "#define _%s 0x%s\n", $$3, $$1}' > $(SYMBOLS)
	g++ -Ibuild -I$(INCLUDE) tools/MemoryMapGenerator.cpp -o $(BUILD)/MemoryMapGenerator $(SYMBOLS)
	$(BUILD)/MemoryMapGenerator $(MEMORY_MAP)
	$(OBJCOPY) --update-section .__MEMORY_MAP__=$(MEMORY_MAP) $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin
	$(OBJCOPY) -O binary $(BUILD)/$(APPLICATION).elf $(BUILD)/$(APPLICATION).bin
	$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios none -nographic -m $(MEMORY)b -device loader,file=$(TARGET).bin,addr=$(BOOT_ADDR),force-raw=on -device loader,file=$(BUILD)/$(APPLICATION).bin,addr=$(APP_ADDR),force-raw=on

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

$(TARGET).elf: $(OBJS)
	./Linker $(BOOT_ADDR) LD_KERNEL > $(LINKER)
	$(LD) -T $(LINKER) -o $@ $(OBJS)

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -MMD -MP -c $< -o $@

clean:
	rm -rf build
	(cd app && make clean)

-include $(DEPS)
