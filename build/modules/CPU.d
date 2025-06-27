build/modules/CPU.o gcm.cache/CPU.gcm: modules/CPU.cpp
build/modules/CPU.o gcm.cache/CPU.gcm: Definitions.c++-module
CPU.c++-module: gcm.cache/CPU.gcm
.PHONY: CPU.c++-module
gcm.cache/CPU.gcm:| build/modules/CPU.o
CXX_IMPORTS += Definitions.c++-module
