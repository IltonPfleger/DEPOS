#include <io/io.hpp>
#include <memory.hpp>
#include <thread.hpp>

Queue<Thread *> Thread::ready;

void Thread::create(Thread *thread, ThreadEntry entry, Priority priority) {
    thread->stack      = reinterpret_cast<uintptr_t>(Memory::kmalloc());
    thread->context.ra = reinterpret_cast<uintptr_t>(exit);
    thread->context.pc = reinterpret_cast<uintptr_t>(entry);
    thread->context.sp = thread->stack + Machine::Memory::Page::SIZE;
    thread->state      = READY;
    ready.insert(thread, priority);
}

void Thread::exit() {
    IO::out("RETURN\n");
    __asm__ volatile("auipc t0, 0");
}

void Thread::dispatch(Thread *next) {
    CPU::Context::dispatch(&next->context);
    // Save Running Context
    // Switch ContextA
    // Load Context
    // Return
}

void Thread::yield() {}
