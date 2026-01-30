include ../Makedefs.mk

INFO := $(BUILD)/$(APPLICATION).info
BIN := $(BUILD)/$(APPLICATION).bin
BUILD := build

ifneq ($(wildcard $(APPLICATION)/Makefile),)
all:
	$(MAKE) -C $(APPLICATION) all
else

SOURCES := $(wildcard $(APPLICATION)/*.cpp)
OBJECTS := $(patsubst $(APPLICATION)/%.cpp,$(BUILD)/$(APPLICATION)/%.o,$(SOURCES))
DEPENDENCIES := $(OBJECTS:.o=.d)

all: $(BIN) $(INFO)

$(BIN): $(INFO)
	$(OBJCOPY) -O binary $< $@

$(INFO): $(BUILD)/$(APPLICATION).o
	@mkdir -p $(dir $@)
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(APPLICATION_ADDR) --image-base=$(APPLICATION_ADDR) -o $@ $<

$(BUILD)/$(APPLICATION).o: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) -e main -r $(OBJECTS) -o $@

$(BUILD)/$(APPLICATION)/%.o: $(APPLICATION)/%.cpp
	@mkdir -p $(dir $@)
	@echo $@
	$(CC) $(MARCH_CCFLAGS) -I../include -MMD -MP -c $< -o $@

-include $(DEPENDENCIES)
endif
