#include <io/io.hpp>
#include <thread.hpp>

Thread::Queue Thread::_ready;
volatile Thread *Thread::_running;

void Thread::Queue::put(Thread *thread) {
    Node *item     = new (HEAP) Node;
    Node *current  = head;
    Node *previous = nullptr;
    item->value    = thread;

    while (current && current->value->priority <= thread->priority) {
        previous = current;
        current  = current->next;
    }

    if (head == nullptr) {
        head = item;
    } else if (current == head) {
        item->next = head;
        head       = item;
    } else if (current == nullptr) {
        previous->next = item;
        item->next     = nullptr;
    } else {
        item->next     = current;
        previous->next = item;
    }
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

void Thread::exit() {
    Thread *previous = const_cast<Thread *>(_running);
    Thread *next     = _ready.get();

    if (next) dispatch(next);

    IO::out("The Last Thread Has Exited!\n");
    while (1);
}

void Thread::dispatch(Thread *next) {
    _running        = next;
    _running->state = RUNNING;
    CPU::Context::relay(&next->context);
}

void Thread::yield() {
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _ready.put(previous);
    Thread *next = _ready.get();
    dispatch(next);
}
