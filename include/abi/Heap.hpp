#pragma once

#include <abi/ABI.hpp>
#include <architecture/Syscall.hpp>
#include <types.hpp>

inline void *operator new(QUARK::size_t s) { return QUARK::Syscall(QUARK::ABI::Function::ABI_HEAP_NEW, s); }
inline void *operator new[](QUARK::size_t s) { return ::operator new(s); }

inline void operator delete(void *p) { QUARK::Syscall(QUARK::ABI::Function::ABI_HEAP_DELETE, p); }
inline void operator delete(void *p, QUARK::size_t) { ::operator delete(p); }
inline void operator delete[](void *p) { ::operator delete(p); }
inline void operator delete[](void *p, QUARK::size_t) { ::operator delete(p); }
