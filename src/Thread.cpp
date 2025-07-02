#include <Thread.hpp>

extern int main(void *);
volatile Thread::Thread *_running;
int _count;
Thread::Queue _ready;
Thread::Thread *_idle_thread;
Thread::Thread *_user_thread;

void dispatch(Thread::Thread *current, Thread::Thread *next) {
    if (current == next) Logger::log("ERROR\n");
    _running        = next;
    _running->state = Thread::RUNNING;
    CPU::Context::transfer(&current->context, next->context);
}

int idle(void *) {
    while (1) {
        if (_count == 1) {
            Thread::stop();
        } else {
            Logger::log("*** OI ***\n");
            CPU::Interrupt::enable();
            CPU::idle();
            // Thread::yield();
        }
    }
    return 0;
}

Thread::Thread::Thread(int (*entry)(void *), void *args, Priority priority) {
    stack   = reinterpret_cast<uintptr_t>(Memory::kmalloc());
    context = reinterpret_cast<CPU::Context *>(stack + Machine::Memory::Page::SIZE);
    context -= sizeof(CPU::Context);
    CPU::Context::create(context, entry, exit, args);
    state    = READY;
    priority = priority;
    _count++;
    _ready.put(this);
}

Thread::Thread::~Thread() {
    switch (state) {
        case (READY):
            _ready.remove(this);
            _count--;
            break;
        case (WAITING):
            waiting->remove(this);
            _count--;
            break;
    }
    Memory::kfree(reinterpret_cast<void *>(stack));
}

void Thread::join(Thread *thread) {
    CPU::Interrupt::disable();
    if (thread->state != FINISHED) {
        Thread *previous = const_cast<Thread *>(_running);
        if (thread == previous) Logger::log("ERROR: Self join detected.");

        previous->state = WAITING;

        if (thread->joining) Logger::log("ERROR: Multiple joins detected.");
        thread->joining = previous;

        dispatch(previous, _ready.get());
    }
    CPU::Interrupt::enable();
}

void Thread::exit() {
    CPU::Interrupt::disable();
    Logger::log("ERROR\n");
    Thread *previous = const_cast<Thread *>(_running);
    if (previous->joining) _ready.put(previous->joining);
    previous->state = FINISHED;

    _count--;
    Thread *next = _ready.get();
    dispatch(previous, next);
}

void Thread::init() {
    _idle_thread    = new (Memory::SYSTEM) Thread(idle, 0, IDLE);
    _user_thread    = new (Memory::SYSTEM) Thread(main, 0, NORMAL);
    Thread *first   = _ready.get();
    _running        = first;
    _running->state = RUNNING;
    CPU::Context::jump(first->context);
}

void Thread::stop() {
    CPU::Interrupt::disable();
    delete _idle_thread;
    delete _user_thread;
    Logger::log("*** The last thread under control of QUARK has finished. ***\n");
    Logger::log("*** QUARK is shutting down! ***\n");
    while (1);
}

void Thread::timer_handler() {
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _ready.put(previous);
    Thread *next    = _ready.get();
    _running        = next;
    _running->state = RUNNING;
    CPU::Context::jump(next->context);
}

void Thread::yield() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    Thread *next     = _ready.get();
    _ready.put(previous);
    dispatch(previous, next);
}

void Thread::sleep(Queue *waiting) {
    Thread *previous  = const_cast<Thread *>(_running);
    previous->state   = WAITING;
    previous->waiting = waiting;
    waiting->put(previous);
    Thread *next = _ready.get();
    dispatch(previous, next);
}

void Thread::wakeup(Queue *waiting) {
    if (Thread *awake = waiting->get()) {
        awake->state   = READY;
        awake->waiting = nullptr;
        _ready.put(awake);
    }
}

void Thread::save(CPU::Context *context) { _running->context = context; }
