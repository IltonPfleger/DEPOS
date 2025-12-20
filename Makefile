include Makedefs.mk

APP=$(APPLICATIONS)/$(BUILD)/$(APPLICATION)
TOOLS := $(patsubst tools/%.cpp,$(BUILD)/%,$(shell find tools -type f -name "*.cpp"))
SRCS := $(shell find src -type f -name "*.cpp")
OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
MAP := $(BUILD)/Map

run: $(IMAGE)
	$(QEMU) -nographic -M $(MACHINE) -smp $(CPUS) -bios none -m $(MEMORY_SIZE)b -kernel $(IMAGE)

debug: $(KERNEL)
	$(QEMU) -nographic -M $(MACHINE) -smp $(CPUS) -bios none -m $(MEMORY_SIZE)b -kernel $(IMAGE) -S -gdb tcp::1234

norun: $(IMAGE)

gdb:
	riscv64-linux-gnu-gdb\
		-ex "target extended-remote:1234"\
		-ex "set confirm off"\
		-ex "add-inferior"\
		-ex "inferior 2"\
		-ex "attach 2"\
		-ex "set confirm off"\
		-ex "file $(KERNEL).elf"
	#-ex "add-symbol-file $(KERNEL).elf $(PhysicalBootAddr)"\

$(IMAGE): $(KERNEL).elf $(TOOLS)
	make APPLICATION=$(APPLICATION) -C $(APPLICATIONS)
	$(LD) -e main --just-symbols $(KERNEL).elf --section-start=.text=$(ApplicationAddr) --image-base=$(ApplicationAddr) -o $(APP).elf $(APP).o
	$(EMAP) $(APP).elf $(MAP)
	$(OBJCOPY) --update-section .__app_mm__=$(MAP) $(KERNEL).elf
	$(OBJCOPY) -O binary $(APP).elf $(APP).bin
	$(EMAP) $(KERNEL).elf $(MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MAP) $(KERNEL).elf
	$(OBJCOPY) -O binary $(KERNEL).elf $(IMAGE)
	$(DD) bs=1 conv=notrunc if=$(APP).bin of=$(IMAGE) seek=$$(( $$(./tools/EPrint $(APP).elf -b) - $(RamStart) ))
	$(TRUNCATE) -s %$(PAGE_SIZE) $(IMAGE)


$(KERNEL).elf: $(OBJS)
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
