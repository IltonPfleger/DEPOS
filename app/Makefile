include ../Makedefs.mk

TARGET := $(BUILD)/$(APPLICATION).elf

SOURCES := $(if $(APPLICATION),$(shell find $(APPLICATION) -type f -name "*.cpp"))
OBJECTS := $(patsubst $(APPLICATION)/%.cpp,$(BUILD)/$(APPLICATION)/%.o,$(SOURCES))
DEPENDENCIES := $(OBJECTS:.o=.d)

$(TARGET): $(BUILD)/$(APPLICATION).o
	$(LD) -e main --just-symbols $(SYSTEM).elf --image-base=$(ApplicationAddr) -o $@ $<

$(BUILD)/$(APPLICATION).o: $(OBJECTS)
	$(LD) -e main -r $(OBJECTS) -o $@

$(BUILD)/$(APPLICATION)/%.o: ./$(APPLICATION)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I../include -MMD -MP -c $< -o $@

clean:
	rm -rf build

-include $(DEPENDENCIES)
