#include <kernel/memory/Memory.hpp>
#include <shared/console/Debug.hpp>
#include <shared/memory/Heap.hpp>

using namespace DEPOS;

void *operator new(unsigned long size) { return ::operator new(size, Heap::Location::SYSTEM); }
void *operator new[](unsigned long size) { return ::operator new(size, Heap::Location::SYSTEM); }
void *operator new[](unsigned long size, Heap::Location location) { return ::operator new(size, location); }
void *operator new(unsigned long, void *p) { return p; }

void *operator new(unsigned long size, Heap::Location) {
    using Header = Heap::Header;
    size += sizeof(Header);
    Header *header = reinterpret_cast<Header *>(Memory::alloc(size));
    header->m_size = size;
    return header + 1;
}

void operator delete(void *p, unsigned long) { ::operator delete(p); }
void operator delete[](void *p) { ::operator delete(p); }
void operator delete[](void *p, unsigned long) { ::operator delete(p); }

void operator delete(void *p) {
    ERROR(!p);
    using Header   = Heap::Header;
    Header *header = reinterpret_cast<Header *>(p) - 1;
    Memory::free(header, header->m_size);
}
