include ../Makedefs.mk

BUILD := build
TARGET := $(BUILD)/$(APPLICATION).elf
APPLICATION ?=

SRCS := $(if $(APPLICATION),$(shell find $(APPLICATION) -type f -name "*.cpp"))
OBJS := $(patsubst $(APPLICATION)/%.cpp,$(BUILD)/$(APPLICATION)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ -e main

$(BUILD)/$(APPLICATION)/%.o: ./$(APPLICATION)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I../include -fPIC -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf build
