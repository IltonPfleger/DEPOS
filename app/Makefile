include ../Makedefs.mk

TARGET := $(BUILD)/$(APPLICATION).elf
APPLICATION ?=

SRCS := $(if $(APPLICATION),$(shell find $(APPLICATION) -type f -name "*.cpp"))
OBJS := $(patsubst $(APPLICATION)/%.cpp,$(BUILD)/$(APPLICATION)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)


$(TARGET): $(OBJS)
	#$(CC) -o $@ $(OBJS) -e main
	#$(LD) -r --just-symbols ../$(KERNEL).elf $(OBJS) -o $@
	$(CC) -nostdlib -Wl,-r,--just-symbols=../$(KERNEL).elf $(OBJS) -o $@
	#$(LD) -r ../$(KERNEL).elf $(OBJS) -o $@

$(BUILD)/$(APPLICATION)/%.o: ./$(APPLICATION)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I../include -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf build
