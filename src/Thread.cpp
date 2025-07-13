#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

extern int main(void *);
static Thread *_idle_thread;
static Thread *_user_thread;

void dispatch(Thread *next) {
    Thread *previous = const_cast<Thread *>(Thread::_running);
    ERROR(next == nullptr, "[Thread::dispatch] Invalid thread.");
    ERROR(previous == next, "[Thread::dispatch] Same thread.");
    Thread::_running        = next;
    Thread::_running->state = Thread::RUNNING;
    CPU::Atomic::unlock(&Thread::_lock);
    CPU::Interrupt::disable();
    CPU::Context::transfer(&previous->context, next->context);
}

int idle(void *) {
    while (1) {
        CPU::Atomic::lock(&Thread::_lock);
        // Logger::println("IDLE %d\n", Thread::_count);
        if (Thread::_count == 1) {
            CPU::Interrupt::disable();
            //Logger::println("HIT> %d\n", Thread::_count);
            CPU::Atomic::unlock(&Thread::_lock);
            delete _idle_thread;
            delete _user_thread;
            CPU::Atomic::lock(&Thread::_lock);
            Logger::println("*** QUARK is shutting down! ***\n");
            // CPU::Atomic::unlock(&Thread::_lock);
            while (1);
        } else {
            // CPU::idle();
            Logger::println("IDLEE\n");
            if (!Thread::_scheduler.empty()) {
                CPU::Atomic::unlock(&Thread::_lock);
                Thread::yield();
            };
        }
        CPU::Atomic::unlock(&Thread::_lock);
    }
    return 0;
}

Thread::Thread(int (*function)(void *), void *args, Priority priority) {
    CPU::Atomic::lock(&_lock);
    stack       = Memory::kmalloc();
    char *entry = reinterpret_cast<char *>(stack);
    entry += Traits<Memory>::Page::SIZE - sizeof(CPU::Context);
    context = new (entry) CPU::Context(function, exit, args);
    state   = READY;
    rank    = priority;
    _count++;
    _scheduler.insert(this);
    CPU::Atomic::unlock(&_lock);
}

Thread::~Thread() {
    CPU::Atomic::lock(&_lock);
    switch (state) {
        case (READY):
            _scheduler.remove(this);
            _count--;
            break;
        case (WAITING):
            // waiting->remove(this);
            _count--;
            break;
        default:
            break;
    }
    CPU::Atomic::unlock(&_lock);
    Memory::kfree(stack);
}

void Thread::join(Thread *thread) {
    CPU::Atomic::lock(&_lock);
    if (thread->state != FINISHED) {
        Thread *previous = const_cast<Thread *>(_running);

        ERROR(thread == previous, "[Thread::join] Join itself.");
        ERROR(thread->joining != nullptr, "[Thread::join] Already joined.");

        previous->state = WAITING;
        thread->joining = previous;
        dispatch(_scheduler.chose());
    }
    CPU::Atomic::unlock(&_lock);
}

void Thread::exit() {
    CPU::Atomic::lock(&_lock);
    Thread *previous = const_cast<Thread *>(_running);
    if (previous->joining) _scheduler.insert(previous->joining);
    previous->state = FINISHED;
    _count--;
    Thread *next = _scheduler.chose();
    dispatch(next);
}

void Thread::init() {
    _idle_thread    = new (Memory::SYSTEM) Thread(idle, 0, IDLE);
    _user_thread    = new (Memory::SYSTEM) Thread(main, 0, NORMAL);
    Thread *first   = _scheduler.chose();
    _running        = first;
    _running->state = RUNNING;
    CPU::Context::jump(first->context);
}

void Thread::reschedule() {
    CPU::Atomic::lock(&_lock);
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _scheduler.insert(previous);
    Thread *next    = _scheduler.chose();
    _running        = next;
    _running->state = RUNNING;
    CPU::Atomic::unlock(&_lock);
}

void Thread::yield() {
    CPU::Atomic::lock(&_lock);
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    Thread *next     = _scheduler.chose();
    _scheduler.insert(previous);
    dispatch(next);
}

// void Thread::sleep(List *waiting) {
//     CPU::Atomic::lock(&_lock);
//     Thread *previous  = const_cast<Thread *>(_running);
//     previous->state   = WAITING;
//     previous->waiting = waiting;
//     waiting->insert(previous);
//     Thread *next = _scheduler.chose();
//     dispatch(previous, next);
//     CPU::Atomic::unlock(&_lock);
// }
//
// void Thread::wakeup(List *waiting) {
//     CPU::Atomic::lock(&_lock);
//     Thread *awake = waiting->next();
//     ERROR(awake == nullptr, "[Thread::wakeup] Empty queue.");
//     awake->state   = READY;
//     awake->waiting = nullptr;
//     _scheduler.insert(awake);
//     CPU::Atomic::unlock(&_lock);
// }

// RT_Thread::RT_Thread(int (*function)(void *), void *args, RT_Thread::Period period)
//     : Thread(function, args, NORMAL), period(period) {}  //, last(Timer::time()) {}
//
// void RT_Thread::wait_next() {
//     volatile RT_Thread *running = reinterpret_cast<volatile RT_Thread *>(_running);
//     Alarm::usleep(running->period);
//     //
//     //    running->last += running->period;
//     //
//     //    RT_Thread::Period now  = Timer::time(); //CONTANDO TICKS E DORMINDO EM MICROSEGUNDOS!!!
//     //    RT_Thread::Period next = running->last;
//     //
//     //    if (now < next) {
//     //        Alarm::udelay(next - now);
//     //    } else {
//     //        Logger::println("Missed deadline by %d us\n", now - next);
//     //    }
// }
