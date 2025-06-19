#include <io/io.hpp>
#include <memory.hpp>
#include <thread.hpp>

Thread::Thread(ThreadFunction function) {
    context.sp = reinterpret_cast<uintptr_t>(Memory::kmalloc()) +
                 Machine::Memory::Page::SIZE;
    context.ra = reinterpret_cast<uintptr_t>(exit);
    context.load();
    __asm__ volatile("jr ra");
}

__attribute__((naked)) void Thread::exit() { IO::out("RETURN"); }
