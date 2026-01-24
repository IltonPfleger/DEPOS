#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

void *operator new(unsigned long size, Heap::Location location) {
    using Header = Heap::Header;
    (void)location;

    Header *header = reinterpret_cast<Header *>(Memory::kmalloc(size + sizeof(Header)));
    header->m_size = size;
    return header + 1;
}

void *operator new(unsigned long size) { return ::operator new(size, Heap::SYSTEM); }
void *operator new[](unsigned long size) { return ::operator new(size, Heap::SYSTEM); }
void *operator new[](unsigned long size, Heap::Location location) { return ::operator new(size, location); }

void operator delete(void *p) {
    ERROR(!p);
    using Header = Heap::Header;
    Header *header = reinterpret_cast<Header *>(p) - 1;
    Memory::kfree(header, header->m_size);
}

void operator delete(void *p, unsigned long) { ::operator delete(p); }
void operator delete[](void *p) { ::operator delete(p); }
void operator delete[](void *p, unsigned long) { ::operator delete(p); }
