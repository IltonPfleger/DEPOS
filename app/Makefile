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
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(APPLICATION_ADDR) --image-base=$(APPLICATION_ADDR) -o $@ $<

$(BUILD)/$(APPLICATION).o: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) -e main -r $(OBJECTS) -o $@

$(BUILD)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(MARCH_CCFLAGS) -I../include -MMD -MP -c $< -o $@

-include $(DEPENDENCIES)
endif
