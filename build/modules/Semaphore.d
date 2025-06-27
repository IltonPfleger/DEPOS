build/modules/Semaphore.o gcm.cache/Semaphore.gcm: modules/Semaphore.cpp
build/modules/Semaphore.o gcm.cache/Semaphore.gcm: Thread.c++-module \
 CPU.c++-module
Semaphore.c++-module: gcm.cache/Semaphore.gcm
.PHONY: Semaphore.c++-module
gcm.cache/Semaphore.gcm:| build/modules/Semaphore.o
CXX_IMPORTS += Thread.c++-module CPU.c++-module
