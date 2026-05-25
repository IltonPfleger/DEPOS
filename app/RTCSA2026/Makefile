include ../../Makedefs.mk

ELF := $(BUILD)/$(APPLICATION).elf
BUILD := $(BUILD)/$(APPLICATION)

SOURCES := $(shell find . -name '*.cpp')
OBJECTS     := $(SOURCES:%=$(BUILD)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

.PHONY: EPOS

all: $(ELF) EPOS

$(ELF): $(BUILD)/$(APPLICATION).o
	@mkdir -p $(dir $@)
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(Application_Addr) --image-base=$(Application_Addr) -o $@ $<

$(BUILD)/$(APPLICATION).o: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) -e main -r $(OBJECTS) -o $@

$(BUILD)/%.cpp.o: %.cpp $(CONFIG) 
	@mkdir -p $(dir $@)
	$(CC) $(MACH_CCFLAGS) -I../include -MMD -MP -c $< -o $@

EPOS:
	./EPOS.sh

-include $(DEPENDENCIES)
