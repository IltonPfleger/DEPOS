MARCH_CCFLAGS += -mcmodel=medany -D __ARCH=riscv64
MARCH_CCFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
MARCH_CCFLAGS += -msmall-data-limit=0 -fno-threadsafe-statics -fno-use-cxa-atexit
