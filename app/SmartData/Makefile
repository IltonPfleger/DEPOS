include ../../Makedefs.mk

INFO := $(BUILD)/$(APPLICATION).info
BIN := $(BUILD)/$(APPLICATION).bin
#BUILD := $(BUILD)/$(APPLICATION)

SOURCES := $(shell find . \( -name '*.cpp' -o -name '*.cc' \))
OBJECTS := $(patsubst ./%, $(BUILD)/$(APPLICATION)/%, $(SOURCES))
OBJECTS := $(OBJECTS:.cpp=.o)
OBJECTS := $(OBJECTS:.cc=.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

all: $(BIN) $(INFO)

$(BIN): $(INFO)
	$(OBJCOPY) -O binary $< $@

$(INFO): $(OBJECTS)
	@mkdir -p $(dir $@)
	#$(LD) -e main --just-symbols $(SYSTEM).elf 
	$(LD) -e main \
	      -Ttext=$(APPLICATION_ADDR) \
	      --image-base=$(APPLICATION_ADDR) \
	      -o $@ $^ -R $(SYSTEM).elf

$(BUILD)/$(APPLICATION)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CC) $(MARCH_CCFLAGS) -Ismartdata/include -I../include \
	      -MMD -MP -c $< -o $@ \
	      -Wno-deprecated-enum-enum-conversion -Wno-overflow \
	      -Wno-unused-parameter -Wno-pedantic \
	      -Wno-tautological-compare -Wno-volatile \
	      -Wno-ignored-qualifiers -Wno-deprecated-copy \
	      -Wno-return-type

$(BUILD)/$(APPLICATION)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(MARCH_CCFLAGS) -Ismartdata/include -I../include \
	      -MMD -MP -c $< -o $@ \
	      -Wno-deprecated-enum-enum-conversion -Wno-overflow \
	      -Wno-unused-parameter -Wno-pedantic \
	      -Wno-tautological-compare -Wno-volatile \
	      -Wno-ignored-qualifiers -Wno-deprecated-copy \
	      -Wno-return-type



-include $(DEPENDENCIES)
