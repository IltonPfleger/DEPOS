#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <utility/Debug.hpp>

using namespace QUARK;

void *operator new(unsigned long size, Heap::Location) {
    using Header   = Heap::Header;
    Header *header = reinterpret_cast<Header *>(Memory::alloc(size + sizeof(Header)));
    header->magic  = Heap::MAGIC;
    header->size   = size + sizeof(Header);
    return header + 1;
}

void *operator new(unsigned long, void *p) { return p; }
void *operator new(unsigned long size) { return ::operator new(size, Heap::SYSTEM); }
void *operator new[](unsigned long size) { return ::operator new(size, Heap::SYSTEM); }
void *operator new[](unsigned long size, Heap::Location location) { return ::operator new(size, location); }

void operator delete(void *p) {
    using Header   = Heap::Header;
    Header *header = reinterpret_cast<Header *>(p) - 1;
    assert(p);
    assert(header);
    assert(header->magic == Heap::MAGIC);
    Memory::free(header, header->size);
}

void operator delete(void *p, unsigned long) { ::operator delete(p); }
void operator delete[](void *p) { ::operator delete(p); }
void operator delete[](void *p, unsigned long) { ::operator delete(p); }
