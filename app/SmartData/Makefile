include ../../Makedefs.mk

INFO := $(BUILD)/$(APPLICATION).info
BIN := $(BUILD)/$(APPLICATION).bin
BUILD := $(BUILD)/$(APPLICATION)

SOURCES := $(wildcard $(APPLICATION)/*.cpp)
OBJECTS := $(patsubst $(APPLICATION)/%.cpp,$(BUILD)/$(APPLICATION)/%.o,$(SOURCES))
DEPENDENCIES := $(OBJECTS:.o=.d)

all: $(BIN) $(INFO)

$(BIN): $(INFO)
	$(OBJCOPY) -O binary $< $@

$(INFO): $(BUILD)/$(APPLICATION).o
	@mkdir -p $(dir $@)
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(APPLICATION_ADDR) --image-base=$(APPLICATION_ADDR) -o $@ $<

$(BUILD)/$(APPLICATION).o: $(APPLICATION).cpp
	@mkdir -p $(dir $@)
	$(CC) $(MARCH_CCFLAGS) -Ismartdata/include -I../include -MMD -MP -c $< -o $@ -Wno-deprecated-enum-enum-conversion -Wno-overflow -Wno-unused-parameter 
	$(LD) -e main -r $(OBJECTS) -o $@

-include $(DEPENDENCIES)
