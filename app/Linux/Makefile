include ../../Makedefs.mk

TARGET_BUILD_DIR := $(BUILD)/$(APPLICATION)

ELF := $(BUILD)/$(APPLICATION).elf

SOURCES := $(shell find . -name '*.cpp')
OBJECTS := $(patsubst %.cpp,$(TARGET_BUILD_DIR)/%.o,$(SOURCES))
DEPENDENCIES := $(OBJECTS:.o=.d)

KERNEL = Image
INITRAMFS = initramfs.cpio
KERNEL_BLOB = $(TARGET_BUILD_DIR)/$(KERNEL).inc
INITRAMFS_BLOB = $(TARGET_BUILD_DIR)/$(INITRAMFS).inc

all: $(ELF)

$(ELF): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(Application_Addr) --image-base=$(Application_Addr) -o $@ $^

$(TARGET_BUILD_DIR)/%.o: %.cpp $(KERNEL_BLOB) $(INITRAMFS_BLOB)
	@mkdir -p $(dir $@)
	$(CC) -D__INITRAMFS__=$(INITRAMFS_BLOB) -D__KERNEL__=$(KERNEL_BLOB) $(MACH_CCFLAGS) -I$(TARGET_BUILD_DIR) -I../include -MMD -MP -c $< -o $@

$(KERNEL_BLOB): $(KERNEL)
	@mkdir -p $(dir $@)
	xxd -i $< | sed '1d' | head -n -2 > $@

$(INITRAMFS_BLOB): $(INITRAMFS)
	@mkdir -p $(dir $@)
	xxd -i $< | sed '1d' | head -n -2 > $@

-include $(DEPENDENCIES)
