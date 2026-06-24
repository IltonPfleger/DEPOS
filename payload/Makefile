include ../Makedefs.mk

ELF := $(BUILD)/$(PAYLOAD).elf
BUILD := $(BUILD)/$(PAYLOAD)

ifneq ($(wildcard $(PAYLOAD)/Makefile),)

all:
	$(MAKE) -C $(PAYLOAD) all

else

SOURCES := $(shell find $(PAYLOAD) -name '*.cpp')
OBJECTS     := $(SOURCES:%=$(BUILD)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

all: $(ELF)

$(ELF): $(BUILD)/$(PAYLOAD).o
	@mkdir -p $(dir $@)
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(Payload_Address) --image-base=$(Payload_Address) -o $@ $<

$(BUILD)/$(PAYLOAD).o: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) -e main -r $(OBJECTS) -o $@

$(BUILD)/%.cpp.o: %.cpp $(CONFIG)
	@mkdir -p $(dir $@)
	$(CC) $(MACH_CCFLAGS) -I../include -MMD -MP -c $< -o $@

-include $(DEPENDENCIES)

endif
