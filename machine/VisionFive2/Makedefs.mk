include $(HERE)/architecture/riscv64/Makedefs.mk
MACH_CCFLAGS += -march=rv64gc_zicsr_zbb -mabi=lp64

%.img: %.bin
	mkdir -p $(dir $@)
	cp -f $< $@
	$(TRUNCATE) -s %4096 $@
