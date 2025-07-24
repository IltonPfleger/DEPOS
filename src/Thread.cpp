#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>

extern int main(void *);
static Thread *_user_thread;

__attribute__((naked)) void dispatch(Thread *previous, Thread *next) {
    CPU::Context::push();
    previous->context = CPU::Context::get();

    ERROR(next == nullptr, "[Thread::dispatch] Invalid thread.");
    ERROR(previous == next, "[Thread::dispatch] Same thread.");
    Thread::running(next);

    Thread::_lock.unlock();
    CPU::Context::jump(next->context);
}

int Thread::idle(void *) {
    while (_count > Machine::CPUS) {
        lock();
        if (!_scheduler.empty())
            yield();
        else
            unlock();
    }

    CPU::Interrupt::disable();
    if (CPU::core() == 0) {
        _lock.lock();
        Logger::println("*** QUARK is shutting down! ***\n");
        _lock.unlock();
    }
    for (;;);
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c) : state(State::READY), criterion(c) {
    joining     = 0;
    stack       = Memory::kmalloc();
    link        = new (Memory::SYSTEM) Element(this, c.priority());
    char *entry = reinterpret_cast<char *>(stack) + Traits::Memory::Page::SIZE - sizeof(CPU::Context);
    context     = new (entry) CPU::Context(f, exit, a);

    lock();
    _scheduler.insert(link);
    _count = _count + 1;
    unlock();
}

Thread::~Thread() {
    switch (state) {
        case (State::READY):
            lock();
            _scheduler.remove(link);
            _count = _count - 1;
            unlock();
            break;
        case (State::WAITING):
            lock();
            waiting->remove(link);
            _count = _count - 1;
            unlock();
            break;
        default:
            break;
    }
    Memory::kfree(stack);
    delete link;
}

void Thread::join(Thread *thread) {
    lock();
    if (thread->state == State::FINISHED) {
        unlock();
        return;
    }
    Thread *previous = const_cast<Thread *>(running());
    ERROR(thread == previous, "[Thread::join] Join itself.");
    ERROR(thread->joining != nullptr, "[Thread::join] Already joined.");
    previous->state = State::WAITING;
    thread->joining = previous;
    Thread *next    = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::exit() {
    Thread *previous = const_cast<Thread *>(running());
    previous->state  = State::FINISHED;

    lock();
    if (previous->joining) {
        previous->joining->state = State::READY;
        _scheduler.insert(previous->joining->link);
        previous->joining = 0;
    }
    _count       = _count - 1;
    Thread *next = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::init() {
    if (CPU::core() == 0) {
        _user_thread = new (Memory::SYSTEM) Thread(main, 0, NORMAL);
    }
    new (Memory::SYSTEM) Thread(idle, 0, IDLE);
}

void Thread::go() {
    while (_count < Machine::CPUS);
    _lock.lock();
    Thread *first = _scheduler.chose();
    running(first);
    _lock.unlock();
    CPU::Context::jump(first->context);
}

void Thread::yield() {
    Thread *previous = const_cast<Thread *>(running());
    previous->state  = State::READY;
    Thread *next     = _scheduler.chose();
    _scheduler.insert(previous->link);
    dispatch(previous, next);
}

void Thread::reschedule() {
    Thread *previous = const_cast<Thread *>(running());
    previous->state  = State::READY;

    _lock.lock();
    _scheduler.insert(previous->link);
    Thread *next = _scheduler.chose();
    running(next);
    _lock.unlock();
}

void Thread::sleep(Queue *waiting) {
    Thread *previous  = const_cast<Thread *>(running());
    previous->state   = State::WAITING;
    previous->waiting = waiting;
    lock();
    waiting->insert(previous->link);
    Thread *next = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::wakeup(Queue *waiting) {
    _lock.lock();
    Element *awake = waiting->next();
    ERROR(awake == nullptr, "[Thread::wakeup] Empty queue.");
    awake->value->state   = State::READY;
    awake->value->waiting = nullptr;
    _scheduler.insert(awake);
    _lock.unlock();
}

// int entry(void *arg) {
//     RT_Thread *current = const_cast<RT_Thread *>(static_cast<volatile RT_Thread *>(Thread::running()));
//     auto now           = Alarm::utime();
//     if (now < current->start) Alarm::usleep(current->start - now);
//
//     while (1) {
//         current->function(arg);
//         now = Alarm::utime();
//
//		int miss = (now - current->start) - current->deadline;
//         ERROR(miss > 0, "Missed deadline: %dμ\n", miss);
//
//         current->start += current->period;
//         Alarm::usleep(current->start - now);
//     }
//     return 0;
// }
//
// RT_Thread::RT_Thread(Function f, Argument a, Microsecond d, Microsecond p, Microsecond c, Microsecond s)
//     : Thread(entry, a, Criterion(d, p, c)), function(f), deadline(d), period(p), duration(c), start(s) {}
