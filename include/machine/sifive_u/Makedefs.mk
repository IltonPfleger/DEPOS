MARCH_CCFLAGS = $(CCFLAGS)
MARCH_CCFLAGS += -D__ARCH=$(ARCH)
MARCH_CCFLAGS += -mcmodel=medany
MARCH_CCFLAGS += -ffreestanding -fno-pic -fno-pie -fno-exceptions -fno-rtti -nostdlib -nostartfiles -mno-relax
MARCH_CCFLAGS += -msmall-data-limit=0 -fno-threadsafe-statics -fno-use-cxa-atexit
MARCH_CCFLAGS += -march=rv64g_zicsr -mabi=lp64
MARCH_CCFLAGS += -g -O3
