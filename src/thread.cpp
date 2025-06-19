#include <io/io.hpp>
#include <memory.hpp>
#include <thread.hpp>

Thread::Thread(ThreadEntry entry) {
    stack      = reinterpret_cast<uintptr_t>(Memory::kmalloc());
    context.ra = reinterpret_cast<uintptr_t>(exit);
    context.pc = reinterpret_cast<uintptr_t>(entry);
    context.sp = stack + Machine::Memory::Page::SIZE;
    // CPU::Context::set(&context);
    // CPU::Context::load();
    //__asm__ volatile("ret");
}

void Thread::exit() {
    IO::out("RETURN\n");
    __asm__ volatile("auipc t0, 0");
}

void Thread::dispatch(Thread *current, Thread *next) {
    CPU::Context::dispatch(&next->context);
    // Save Running Context
    // Switch ContextA
    // Load Context
    // Return
}
