build/modules/Memory.o gcm.cache/Memory.gcm: modules/Memory.cpp
build/modules/Memory.o gcm.cache/Memory.gcm: Logger.c++-module \
 Definitions.c++-module
Memory.c++-module: gcm.cache/Memory.gcm
.PHONY: Memory.c++-module
gcm.cache/Memory.gcm:| build/modules/Memory.o
CXX_IMPORTS += Logger.c++-module Definitions.c++-module
