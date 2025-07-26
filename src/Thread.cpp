#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>

extern int main(void *);
static Thread *_user_thread;

__attribute__((naked)) static void dispatch(Thread *previous, Thread *next) {
    CPU::Context::push();
    previous->context = CPU::Context::get();
    next->state       = Thread::State::RUNNING;
    Thread::_lock.unlock();

    ERROR(next == nullptr, "[Thread::dispatch] Invalid thread.");
    ERROR(previous == next, "[Thread::dispatch] Same thread.");
    CPU::Context::jump(next->context);
}

int Thread::idle(void *) {
    while (_count > Machine::CPUS) {
        lock();
        TRACE("[Thread::idle]\n");
        unlock();

        // Thread *next = _scheduler.chose();

        // if (next) {
        //     Thread *previous = const_cast<Thread *>(running());
        //     previous->state  = State::READY;
        //     _scheduler.insert(previous->link);
        //     dispatch(previous, next);
        // } else {
        //     unlock();
        // }
    }

    CPU::Interrupt::disable();
    if (CPU::core() == 0) Logger::println("*** QUARK Shutdown! ***\n");
    for (;;) CPU::idle();
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c) : state(State::READY), criterion(c) {
    stack       = Memory::kmalloc();
    joining     = 0;
    link        = new (Memory::SYSTEM) Element(this, c.priority());
    char *entry = reinterpret_cast<char *>(stack) + Traits::Memory::Page::SIZE - sizeof(CPU::Context);
    context     = new (entry) CPU::Context(f, exit, this, a);

    lock();
    _scheduler.insert(link);
    _count = _count + 1;
    unlock();
}

Thread::~Thread() {
    lock();
    switch (state) {
        case (State::READY):
            _scheduler.remove(link);
            _count = _count - 1;
            break;
        case (State::WAITING):
            waiting->remove(link);
            _count = _count - 1;
            break;
        default:
            break;
    }
    unlock();
    Memory::kfree(stack);
    delete link;
}

void Thread::join(Thread *thread) {
    ERROR(thread == nullptr, "[Thread::join] Invalid thread.");
    lock();

    Thread *previous = const_cast<Thread *>(running());
    if (thread->state == State::FINISHED) {
        unlock();
        return;
    }

    ERROR(thread == previous, "[Thread::join] Join itself.");
    ERROR(thread->joining != nullptr, "[Thread::join] Already joined.");

    previous->state = State::WAITING;
    thread->joining = previous;
    Thread *next    = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::exit() {
    lock();

    Thread *previous = const_cast<Thread *>(running());
    previous->state  = State::FINISHED;
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
    if (CPU::core() == 0) _user_thread = new (Memory::SYSTEM) Thread(main, 0, NORMAL);
    new (Memory::SYSTEM) Thread(idle, 0, IDLE);
}

void Thread::run() {
    lock();
    Thread *first = _scheduler.chose();
    first->state  = State::RUNNING;
    _lock.unlock();
    CPU::Context::jump(first->context);
}

void Thread::reschedule() {
    lock();
    Thread *previous = const_cast<Thread *>(running());
    previous->state  = State::READY;
    _scheduler.insert(previous->link);
    Thread *next = _scheduler.chose();
    next->state  = State::RUNNING;
    _lock.unlock();
    CPU::thread(next);
}

// void Thread::sleep(Queue *waiting) {
//     lock();
//
//     Thread *previous  = const_cast<Thread *>(running());
//     previous->state   = State::WAITING;
//     previous->waiting = waiting;
//     waiting->insert(previous->link);
//     Thread *next = _scheduler.chose();
//     dispatch(previous, next);
// }
//
// void Thread::wakeup(Queue *waiting) {
//     lock();
//
//     Element *awake = waiting->next();
//     ERROR(awake == nullptr, "[Thread::wakeup] Empty queue.");
//     awake->value->state   = State::READY;
//     awake->value->waiting = nullptr;
//     _scheduler.insert(awake);
//     _lock.unlock();
// }

// int entry(void *arg) {
//     RT_Thread *current = const_cast<RT_Thread *>(static_cast<volatile RT_Thread *>(Thread::running()));
//     auto now           = Alarm::utime();
//     if (now < current->start) Alarm::usleep(current->start - now);
//
//     while (1) {
//         current->function(arg);
//         now = Alarm::utime();
//
//         int miss = (now - current->start) - current->deadline;
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
