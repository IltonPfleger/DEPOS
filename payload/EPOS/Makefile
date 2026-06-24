include ../../Makedefs.mk

TARGET_BUILD_DIR := $(BUILD)/$(PAYLOADLICATION)

ELF := $(BUILD)/$(PAYLOADLICATION).elf

SOURCES := $(shell find . -name '*.cpp')
OBJECTS := $(patsubst %.cpp,$(TARGET_BUILD_DIR)/%.o,$(SOURCES))
DEPENDENCIES := $(OBJECTS:.o=.d)

.PHONY: EPOS

KERNEL = EPOS
KERNEL_BLOB = $(TARGET_BUILD_DIR)/$(EPOS).inc

all: $(ELF)

$(ELF): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(Payload_Address) --image-base=$(Payload_Address) -o $@ $^

$(TARGET_BUILD_DIR)/%.o: %.cpp $(KERNEL_BLOB)
	@mkdir -p $(dir $@)
	$(CC) -D__EPOS__=$(KERNEL_BLOB) -I$(TARGET_BUILD_DIR) -I../include -MMD -MP -c $< -o $@ $(MACH_CCFLAGS)

$(KERNEL_BLOB): /srv/tftp/RemoteBootVisionFive2EPOS
	@mkdir -p $(dir $@)
	xxd -i $< | sed '1d' | head -n -2 > $@

/srv/tftp/RemoteBootVisionFive2EPOS: EPOS

EPOS:
#	./EPOS.sh


-include $(DEPENDENCIES)
