include ../Makedefs.mk

ifneq ($(wildcard $(PAYLOAD)/Makefile),)
all:
	$(MAKE) -C $(PAYLOAD) all
else
include Template.mk
endif
