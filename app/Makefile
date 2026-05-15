include ../Makedefs.mk

ELF := $(BUILD)/$(APPLICATION).elf
BUILD := $(BUILD)/$(APPLICATION)

ifneq ($(wildcard $(APPLICATION)/Makefile),)

all:
	$(MAKE) -C $(APPLICATION) all

else

SOURCES := $(shell find $(APPLICATION) -name '*.cpp')
OBJECTS     := $(SOURCES:%=$(BUILD)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

all: $(ELF)

$(ELF): $(BUILD)/$(APPLICATION).o
	@mkdir -p $(dir $@)
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(Application_Addr) --image-base=$(Application_Addr) -o $@ $<

$(BUILD)/$(APPLICATION).o: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) -e main -r $(OBJECTS) -o $@

$(BUILD)/%.cpp.o: %.cpp $(CONFIG)
	@mkdir -p $(dir $@)
	$(CC) $(MACH_CCFLAGS) -I../include -MMD -MP -c $< -o $@

-include $(DEPENDENCIES)

endif
