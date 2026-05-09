include ../../Makedefs.mk

ELF := $(BUILD)/$(APPLICATION).elf
BIN := $(BUILD)/$(APPLICATION).bin
BUILD := $(BUILD)/$(APPLICATION)

SOURCES := $(wildcard $(APPLICATION)/*.cpp)
OBJECTS := $(patsubst $(APPLICATION)/%.cpp,$(BUILD)/$(APPLICATION)/%.o,$(SOURCES))
DEPENDENCIES := $(OBJECTS:.o=.d)

DTB = guest.dtb
KERNEL = Image
INITRAMFS = initramfs.cpio
DTB_BLOB = $(BUILD)/$(DTB).inc
KERNEL_BLOB = $(BUILD)/$(KERNEL).inc
INITRAMFS_BLOB = $(BUILD)/$(INITRAMFS).inc
INITRAMFS_ADDR=$(shell cat Traits.hpp | grep "DTB =" | sed -E 's/.*= (0x[0-9A-Fa-f]+);.*/\1/')

all: $(BIN)

$(BIN): $(ELF)
	@mkdir -p $(dir $@)
	$(OBJCOPY) -O binary $< $@

$(ELF): $(BUILD)/$(APPLICATION).o
	@mkdir -p $(dir $@)
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(Application_Addr) --image-base=$(Application_Addr) -o $@ $<

$(BUILD)/$(APPLICATION).o: $(APPLICATION).cpp $(KERNEL_BLOB) $(DTB_BLOB) $(INITRAMFS_BLOB)
	@mkdir -p $(dir $@)
	$(CC) -D__INITRAMFS__=$(INITRAMFS_BLOB) -D__KERNEL__=$(KERNEL_BLOB) -D__DTB__=$(DTB_BLOB) $(MARCH_CCFLAGS) -I$(BUILD) -I../include -MMD -MP -c $< -o $@

$(DTB_BLOB): $(DTB)
	@mkdir -p $(dir $@)
	#dtc -I dts -O dtb -o guest.dtb guest.dts
	xxd -i $< | sed '1d' | head -n -2 > $@

$(KERNEL_BLOB): $(KERNEL)
	@mkdir -p $(dir $@)
	xxd -i $< | sed '1d' | head -n -2 > $@

$(INITRAMFS_BLOB): $(INITRAMFS)
	@mkdir -p $(dir $@)
	xxd -i $< | sed '1d' | head -n -2 > $@

-include $(DEPENDENCIES)
