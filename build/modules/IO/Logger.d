build/modules/IO/Logger.o gcm.cache/Logger.gcm: modules/IO/Logger.cpp
build/modules/IO/Logger.o gcm.cache/Logger.gcm: UART.c++-module \
 Definitions.c++-module
Logger.c++-module: gcm.cache/Logger.gcm
.PHONY: Logger.c++-module
gcm.cache/Logger.gcm:| build/modules/IO/Logger.o
CXX_IMPORTS += UART.c++-module Definitions.c++-module
