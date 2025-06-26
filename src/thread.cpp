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

void Thread::create(Thread *thread, int (*entry)(void *), void *arg, Priority priority) {
    thread->stack   = reinterpret_cast<uintptr_t>(Memory::kmalloc());
    thread->context = reinterpret_cast<CPU::Context *>(thread->stack + Machine::Memory::Page::SIZE);
    thread->context -= sizeof(CPU::Context);
    CPU::Context::create(thread->context, entry, exit, arg);
    thread->state    = READY;
    thread->priority = priority;
    _ready.put(thread);
}

void Thread::join(Thread *thread) {
    CPU::Interrupt::disable();
    if (thread->state != FINISHED) {
        Thread *previous = const_cast<Thread *>(_running);
        if (thread == previous) Logger::log("ERROR: Self join detected.");

        previous->state = WAITING;

        if (thread->joining) Logger::log("ERROR: Multiple joins detected.");
        thread->joining = previous;

        Thread::dispatch(previous, _ready.get());
    }

    Memory::kfree(reinterpret_cast<void *>(thread->stack));
    CPU::Interrupt::enable();
}

void Thread::exit() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    if (previous->joining) _ready.put(previous->joining);
    previous->state = FINISHED;

    Thread *next = _ready.get();
    dispatch(previous, next);
}

int Thread::idle(void *ptr) {
    CPU::Interrupt::disable();
    Thread *app = reinterpret_cast<Thread *>(ptr);
    Memory::kfree(reinterpret_cast<void *>(_running->stack));
    Memory::kfree(reinterpret_cast<void *>(app->stack));
    Logger::log("*** The last thread under control of QUARK has finished. ***\n");
    Logger::log("*** QUARK is shutting down! ***\n");
    while (1);
    return 0;
}

void Thread::init() {
    static Thread idle;
    static Thread app;
    Thread::create(&app, main, 0, Thread::Priority::NORMAL);
    Thread::create(&idle, Thread::idle, &app, Thread::Priority::IDLE);
    Thread *first   = Thread::_ready.get();
    _running        = first;
    _running->state = RUNNING;
    CPU::Context::load(first->context);
    CPU::Context::jump();
}

void Thread::reschedule() {
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _ready.put(previous);
    Thread *next    = _ready.get();
    _running        = next;
    _running->state = RUNNING;
    CPU::Context::load(next->context);
    CPU::Context::jump();
}

void Thread::dispatch(Thread *current, Thread *next) {
    _running        = next;
    _running->state = RUNNING;
    CPU::Context::transfer(&current->context, next->context);
}

void Thread::yield() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _ready.put(previous);
    Thread *next = _ready.get();
    dispatch(previous, next);
}

void Thread::sleep(Queue *waiting) {
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = WAITING;
    waiting->put(previous);
    Thread *next = _ready.get();
    dispatch(previous, next);
}

void Thread::wakeup(Queue *waiting) {
    if (Thread *awake = waiting->get()) _ready.put(awake);
}
