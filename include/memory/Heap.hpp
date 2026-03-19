#pragma once

#include <Traits.hpp>

namespace DEPOS {

class Heap {
  public:
    struct Header {
        unsigned long m_size;
    };

    enum Location { SYSTEM };
};

} // namespace DEPOS

void *operator new(unsigned long);
void *operator new(unsigned long, DEPOS::Heap::Location);

void *operator new[](unsigned long);
void *operator new[](unsigned long, DEPOS::Heap::Location);

void *operator new(unsigned long, void *);

void operator delete(void *);
void operator delete(void *, unsigned long);

void operator delete[](void *, unsigned long);
void operator delete[](void *);
