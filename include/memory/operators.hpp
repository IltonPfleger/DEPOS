#pragma once

inline void *operator new(unsigned long, void *pointer) { return pointer; }
