include Makedefs.mk

BUILD := build
TARGET := $(BUILD)/DEPOS
LINKER=linker.ld
SRCS := $(shell find src -type f -name "*.cpp")
OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

CPUS=$(shell ./Meta get $(TRAITS) Traits::Machine::CPUS)
MACHINE=$(shell ./Meta get $(TRAITS) Traits::Machine::NAME)
BOOT_ADDR=$(shell ./Meta get $(TRAITS) Traits::System::ADDR)
APP_ADDR=$(shell ./Meta get $(TRAITS) Traits::Application::ADDR)

run: $(TARGET)
	(cd app && make APPLICATION=$(APPLICATION))
	( \
		TMP=$$(mktemp); \
		cat $(TARGET) app/build/$(APPLICATION).elf > $$TMP; \
		$(QEMU) -M $(MACHINE) -smp $(CPUS) -bios none -nographic -m 1024 -device loader,file=$$TMP,addr=$(BOOT_ADDR),force-raw=on\
	)

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

$(TARGET): $(OBJS)
	@touch $(LINKER)
	@./Meta linker $LINKER $(BOOT_ADDR)
	@$(LD) -T $LINKER -o $@.elf $(OBJS)
	@rm -f $LINKER
	@$(OBJCOPY) -O binary --set-section-flags .bss=alloc,load,contents $@.elf $(TARGET)

$(BUILD)/%.o: src/%.cpp 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Iinclude -MMD -MP -c $< -o $@

clean:
	rm -rf build
	(cd app && make clean)

-include $(DEPS)
