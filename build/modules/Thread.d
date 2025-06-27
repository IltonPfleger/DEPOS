build/modules/Thread.o gcm.cache/Thread.gcm: modules/Thread.cpp
build/modules/Thread.o gcm.cache/Thread.gcm: Memory.c++-module \
 Logger.c++-module CPU.c++-module Definitions.c++-module
Thread.c++-module: gcm.cache/Thread.gcm
.PHONY: Thread.c++-module
gcm.cache/Thread.gcm:| build/modules/Thread.o
CXX_IMPORTS += Memory.c++-module Logger.c++-module CPU.c++-module \
 Definitions.c++-module
