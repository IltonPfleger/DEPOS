include ../../Makedefs.mk

ELF := $(BUILD)/EPOS.elf
BUILD := $(BUILD)/EPOS
KERNEL := $(BUILD)/EPOS

SOURCES := $(shell find . -name '*.cpp')
OBJECTS     := $(SOURCES:%=$(BUILD)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

all: $(ELF)

$(ELF): $(BUILD)/EPOS.o
	@mkdir -p $(dir $@)
	$(LD) -e main --just-symbols $(SYSTEM).elf -Ttext=$(APPLICATION_ADDR) --image-base=$(APPLICATION_ADDR) -o $@ $^

$(BUILD)/EPOS.o: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) -e main -r $(OBJECTS) -o $@

$(BUILD)/%.cpp.o: %.cpp $(KERNEL)
	@mkdir -p $(dir $@)
	$(CC) $(MARCH_CCFLAGS) -I../include -I$(BUILD) -MMD -MP -c $< -o $@ -D__KERNEL__=$(KERNEL)

$(KERNEL):
	@mkdir -p $(dir $@)
	make -C EPOS veryclean APPLICATION=
	make -C EPOS APPLICATION=philosophers_dinner
	$(OBJCOPY) -O binary EPOS/img/philosophers_dinner EPOS/img/philosophers_dinner.bin
	sudo rm /srv/tftp/EPOS.bin
	sudo mv EPOS/img/philosophers_dinner.bin /srv/tftp/EPOS.bin
	#xxd -i EPOS/img/philosophers_dinner.bin | sed '1d' | head -n -2 > $@

clean:
	rm -rf $(BUILD)

-include $(DEPENDENCIES)
