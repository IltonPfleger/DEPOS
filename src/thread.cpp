#include <io/logger.hpp>
#include <thread.hpp>

Thread::Queue Thread::_ready;
volatile Thread *Thread::_running;
extern int main(void *);

void Thread::Queue::put(Thread *thread) {
    Node *item  = new (HEAP) Node;
    item->value = thread;
    item->next  = nullptr;

    if (!head || thread->priority < head->value->priority) {
        item->next = head;
        head       = item;
        return;
    }

    Node *current = head;
    while (current->next && current->next->value->priority <= thread->priority) {
        current = current->next;
    }

    item->next    = current->next;
    current->next = item;
}

Thread *Thread::Queue::get() {
    Node *node = head;
    if (node == nullptr) {
        return 0;
    }
    head           = head->next;
    Thread *thread = node->value;
    operator delete(node, HEAP);
    return thread;
}

void Thread::create(Thread *thread, Entry entry, Priority priority) {
    thread->stack      = reinterpret_cast<uintptr_t>(Memory::kmalloc());
    thread->context.ra = reinterpret_cast<uintptr_t>(exit);
    thread->context.pc = reinterpret_cast<uintptr_t>(entry);
    thread->context.sp = thread->stack + Machine::Memory::Page::SIZE;
    thread->state      = READY;
    thread->priority   = priority;
    _ready.put(thread);
}

void Thread::join(Thread *thread) {
    CPU::Trap::Interrupt::disable();
    if (thread->state != FINISHED) {
        Thread *previous = const_cast<Thread *>(_running);
        if (thread == previous) Logger::log("ERROR: Self join detected.");

        previous->state = WAITING;

        if (thread->joining) Logger::log("ERROR: Multiple joins detected.");
        thread->joining = previous;

        Thread::dispatch(_ready.get());
    }

    Memory::kfree(reinterpret_cast<void *>(thread->stack));
    CPU::Trap::Interrupt::enable();
}

void Thread::exit() {
    CPU::Trap::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    if (previous->joining) _ready.put(previous->joining);
    previous->state = FINISHED;

    Thread *next = _ready.get();
    dispatch(next);
}

int Thread::idle(void *) {
    CPU::Trap::Interrupt::disable();
    // Memory::kfree(reinterpret_cast<void *>(_running->stack));
    Logger::log("*** The last thread under control of QUARK has finished. ***\n");
    Logger::log("*** QUARK is shutting down! ***\n");
    while (1);
    return 0;
}

void Thread::init() {
    static Thread idle;
    static Thread app;
    Thread::create(&app, main, Thread::Priority::NORMAL);
    Thread::create(&idle, Thread::idle, Thread::Priority::IDLE);

    Thread *first = Thread::_ready.get();
    first->state  = RUNNING;
    _running      = first;
    CPU::Context::set(&first->context);
    CPU::Context::dispatch(&first->context);
}

void Thread::reschedule() {
    CPU::Trap::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _ready.put(previous);
    Thread *next    = _ready.get();
    _running        = next;
    _running->state = RUNNING;
    CPU::Context::dispatch(&next->context);
}

void Thread::dispatch(Thread *next) {
    _running        = next;
    _running->state = RUNNING;
    CPU::Context::save();
    CPU::Context::dispatch(&next->context);
}

void Thread::yield() {
    CPU::Trap::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _ready.put(previous);
    Thread *next = _ready.get();
    dispatch(next);
}
