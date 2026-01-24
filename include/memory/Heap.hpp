#pragma once

#include <Traits.hpp>

class Heap {
  public:
    struct Header {
        unsigned int m_size;
    };

    enum Location { SYSTEM };
};

void *operator new(unsigned long);
void *operator new(unsigned long, Heap::Location);

void *operator new[](unsigned long);
void *operator new[](unsigned long, Heap::Location);

void operator delete(void *);
void operator delete(void *, unsigned long);

void operator delete[](void *, unsigned long);
void operator delete[](void *);
