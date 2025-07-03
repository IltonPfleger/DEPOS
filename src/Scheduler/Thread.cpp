#include <IO/Debug.hpp>
#include <Scheduler/Scheduler.hpp>
#include <Scheduler/Thread.hpp>

extern int main(void *);
volatile Thread::Thread *_running;
static int _count;
static Scheduler<Thread::Thread, Thread::Priority::MAX> _scheduler;
static Thread::Thread *_idle_thread;
static Thread::Thread *_user_thread;

void dispatch(Thread::Thread *current, Thread::Thread *next) {
    ERROR(next == nullptr, "[Thread::dispatch] Cannot dispatch an invalid thread.");
    ERROR(current == next, "[Thread::dispatch] Cannot dispatch the same thread.");
    _running        = next;
    _running->state = Thread::RUNNING;
    CPU::Context::transfer(&current->context, next->context);
}

int idle(void *) {
    while (1) {
        if (_count == 1) {
            Thread::stop();
        } else {
            CPU::Interrupt::enable();
            CPU::idle();
        }
    }
    return 0;
}

Thread::Thread::Thread(int (*entry)(void *), void *args, Priority p) {
    stack   = reinterpret_cast<uintptr_t>(Memory::kmalloc());
    context = reinterpret_cast<CPU::Context *>(stack + Machine::Memory::Page::SIZE);
    context -= sizeof(CPU::Context);
    CPU::Context::create(context, entry, exit, args);
    state    = READY;
    priority = p;
    _count++;
    _scheduler.put(this);
}

Thread::Thread::~Thread() {
    switch (state) {
        case (READY):
            _scheduler.remove(this);
            _count--;
            break;
        case (WAITING):
            waiting->remove(this);
            _count--;
            break;
        default:
            break;
    }
    Memory::kfree(reinterpret_cast<void *>(stack));
}

void Thread::join(Thread *thread) {
    CPU::Interrupt::disable();
    if (thread->state != FINISHED) {
        Thread *previous = const_cast<Thread *>(_running);

        ERROR(thread == previous, "[Thread::join] Thread cannot join itself.");
        ERROR(thread->joining != nullptr, "[Thread::join] Thread already has a joining thread.");

        previous->state = WAITING;
        thread->joining = previous;
        dispatch(previous, _scheduler.chose());
    }
    CPU::Interrupt::enable();
}

void Thread::exit() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    if (previous->joining) _scheduler.put(previous->joining);
    previous->state = FINISHED;

    _count--;
    Thread *next = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::init() {
    _idle_thread    = new (Memory::SYSTEM) Thread(idle, 0, IDLE);
    _user_thread    = new (Memory::SYSTEM) Thread(main, 0, NORMAL);
    Thread *first   = _scheduler.chose();
    _running        = first;
    _running->state = RUNNING;
    CPU::Context::jump(first->context);
}

void Thread::stop() {
    CPU::Interrupt::disable();
    delete _idle_thread;
    delete _user_thread;
    Logger::log("*** QUARK is shutting down! ***\n");
    while (1);
}

void Thread::timer_handler() {
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _scheduler.put(previous);
    Thread *next    = _scheduler.chose();
    _running        = next;
    _running->state = RUNNING;
    CPU::Context::jump(next->context);
}

void Thread::yield() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    Thread *next     = _scheduler.chose();
    _scheduler.put(previous);
    dispatch(previous, next);
}

void Thread::sleep(Queue *waiting) {
    Thread *previous  = const_cast<Thread *>(_running);
    previous->state   = WAITING;
    previous->waiting = waiting;
    waiting->put(previous);
    Thread *next = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::wakeup(Queue *waiting) {
    Thread *awake = waiting->get();
    ERROR(awake == nullptr, "[Thread::wakeup]  Cannot wake up a thread from an empty waiting queue.");
    awake->state   = READY;
    awake->waiting = nullptr;
    _scheduler.put(awake);
}

void Thread::save(CPU::Context *context) { _running->context = context; }
