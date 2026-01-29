include Makedefs.mk

IMAGE := $(BUILD)/Image

ifeq ($(Hypervisor),0)
PAYLOAD=$(BUILD)/$(APPLICATION).bin
else
PAYLOAD=$(APPLICATION)
endif

TOOLS := $(patsubst tools/%.cpp,$(BUILD)/%,$(shell find tools -type f -name "*.cpp"))
SRCS := $(shell find src -type f -name "*.cpp")
OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
MAP := $(BUILD)/MemoryMap

norun: $(IMAGE)

run: $(IMAGE)
	-$(QEMU) -M $(MachineName) -smp $(CPUS) -bios none -nographic -m $(MemorySize)b -kernel $(IMAGE) \
		-device loader,file=guest.dtb,addr=0x82200000,force-raw=on \
		-device loader,file=initramfs.cpio,addr=0x84000000,force-raw=on
	#-$(QEMU) -M $(MachineName) -smp $(CPUS) -bios none -nographic -m $(MemorySize)b -kernel $(IMAGE)

debug: $(IMAGE)
	-$(QEMU) -M $(MachineName) -smp $(CPUS) -bios none -nographic -m $(MemorySize)b -kernel $(IMAGE) -device loader,file=guest.dtb,addr=0x82200000,force-raw=on -S -gdb tcp::1234
	#-$(QEMU) -M $(MachineName) -smp $(CPUS) -bios none -nographic -m $(MemorySize)b -kernel $(SYSTEM).bin -S -gdb tcp::1234

gdb:
	riscv64-linux-gnu-gdb\
		-ex "target extended-remote:1234"\
		-ex "file ../linux/vmlinux"\
		-ex "file build/DEPOS.elf"
		#-ex "set confirm off"\
		#-ex "add-inferior"\
		#-ex "inferior 2"\
		#-ex "attach 2"\
		#-ex "set confirm off"
		#-ex "file $(SYSTEM).elf"

$(IMAGE): $(SYSTEM).bin $(PAYLOAD)
	$(DD) bs=1M conv=notrunc if=$(SYSTEM).bin of=$(IMAGE)
	$(DD) bs=1M conv=notrunc if=$(PAYLOAD) of=$(IMAGE) oflag=seek_bytes seek=$$(( $(ApplicationAddr) - $(RamStart) ))
	$(TRUNCATE) -s %$(PageSize) $(SYSTEM).bin

$(SYSTEM).bin : $(SYSTEM).elf $(TOOLS)
ifeq ($(Hypervisor),0)
ifdef APPLICATION
	make APPLICATION=$(APPLICATION) -C $(APPLICATIONS)
	$(ELFMAP) $(BUILD)/$(APPLICATION).elf $(MAP)
	$(OBJCOPY) --update-section .__app_mm__=$(MAP) $(SYSTEM).elf
	$(OBJCOPY) -O binary $(BUILD)/$(APPLICATION).elf $(PAYLOAD)
endif
else
endif
	$(ELFMAP) $(SYSTEM).elf $(MAP)
	$(OBJCOPY) --update-section .__kernel_mm__=$(MAP) $(SYSTEM).elf
	$(OBJCOPY) -O binary $(SYSTEM).elf $(SYSTEM).bin

$(SYSTEM).elf: $(OBJS)
	$(LD) -e _init --section-start=.init=$(SystemAddr) --image-base=$(SystemAddr) -o $@ $(OBJS)

$(BUILD)/%: tools/%.cpp 
	mkdir -p $(dir $@)
	g++ -std=c++20 -D__MACHINE=$(MachineName) -Iinclude -o $@ $<

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -MMD -MP -c $< -o $@

clean:
	rm -rf build
	(cd app && make clean)

-include $(DEPS)
