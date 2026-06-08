MACH_CCFLAGS += -mcmodel=medany
MACH_CCFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
MACH_CCFLAGS += -msmall-data-limit=0 -fno-threadsafe-statics -fno-use-cxa-atexit -lgcc
