include ../Makedefs.mk

TARGET := $(BUILD)/$(APPLICATION).elf
APPLICATION ?=

SRCS := $(if $(APPLICATION),$(shell find $(APPLICATION) -type f -name "*.cpp"))
OBJS := $(patsubst $(APPLICATION)/%.cpp,$(BUILD)/$(APPLICATION)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	#$(LD) $(OBJS) -o $@ --just-symbols ../$(KERNEL).elf -e main
	$(LD) -r $(OBJS) -o ../$@

$(BUILD)/$(APPLICATION)/%.o: ./$(APPLICATION)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I../include -MMD -MP -c $< -o $@

clean:
	rm -rf build

-include $(DEPS)
