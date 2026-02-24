#pragma once
#include <unistd.h>
#include <pthread.h>
#include <unordered_map>
#include <system/meta.h>
#include <utility/debug.h>
#include <system/types.h>
#include <machine/nic.h>
#include <signal.h>

class Thread
{
public:
	Thread(void* (* function)(void*)) {
		db<Thread>(TRC) << "Thread::Thread()" << endl;

		pthread_attr_t attr;
		pthread_attr_init(&attr);

		pthread_create(&_thread_handle, &attr, function, 0);
		_pthread_thread_umap->insert({_thread_handle, this});

		pthread_attr_destroy(&attr);
	}

	Thread(void* (* function)(void*), void * arg)
	{
		db<Thread>(TRC) << "Thread::Thread()" << endl;

		pthread_attr_t attr;
		pthread_attr_init(&attr);

		pthread_create(&_thread_handle, &attr, function, arg);
		_pthread_thread_umap->insert({_thread_handle, this});

		pthread_attr_destroy(&attr);
	}

	~Thread() {
		_pthread_thread_umap->erase(_thread_handle);
		pthread_kill(_thread_handle, SIGTERM);
		pthread_join(_thread_handle, nullptr); // nullptr for NULL
	}

	static void handler(int id) {
		pthread_exit(0);
	}

	static void assignhandler() {
		static struct sigaction sa;
		sa.sa_handler = handler;
		sigfillset(&sa.sa_mask);
		sigaction(SIGTERM, &sa, nullptr); // nullptr for NULL
	}

	static void yield()
	{
		db<Thread>(TRC) << "Thread::yield()" << endl;
		//usleep(100 * 1000); // TCB - avoid using excessive CPU. Maybe it should be removed at the end of implementation.
		sched_yield();
	}

	static Thread* running() {
		pthread_t thread_handle = pthread_self();
		return _pthread_thread_umap->find(thread_handle)->second;
	}

	static void init();
	static void finish();

private:
	pthread_t _thread_handle;
	static std::unordered_map<pthread_t, Thread *> * _pthread_thread_umap;
};

class Periodic_Thread : public Thread
{
public:
	template<typename ... Tn>
	Periodic_Thread(Microsecond p, Microsecond d, Microsecond delay, void* (* function)(Tn ...), Tn ... an)
    : Thread(function, an ...), _period(p), _deadline(d), _next_activation(TSC::time_stamp()+_period)
	{
		db<Periodic_Thread>(TRC) << "Periodic_Thread::Periodic_Thread(p=" << _period << ",d=" << _deadline << ",act=" << _next_activation << ")" << endl;
	}

	template<typename ... Tn>
	Periodic_Thread(Microsecond p, Microsecond delay, void* (* function)(Tn ...), Tn ... an)
    : Thread(function, an ...), _period(p), _deadline(p), _next_activation(TSC::time_stamp()+_period+delay)
	{
		db<Periodic_Thread>(TRC) << "Periodic_Thread::Periodic_Thread(p=" << _period << ",d=" << _deadline << ",act=" << _next_activation << ")" << endl;
	}

	const Microsecond& period() { return _period; }
	void period(const Microsecond& p) { _period = p; }

	const Microsecond& deadline() { return _deadline; }
	void deadline(const Microsecond& d) { _deadline = d; }

	static volatile bool wait_next(Microsecond activation = 0) {
		db<Periodic_Thread>(TRC) << "Periodic_Thread::wait_next()" << endl;
		Periodic_Thread *t = reinterpret_cast<Periodic_Thread *>(Thread::running());

		if (activation > 0)
			t->_next_activation = activation;

		Microsecond now = TSC::time_stamp();
		if (t->_next_activation > now) {
			usleep(t->_next_activation - now);
			t->_next_activation = t->_next_activation + t->_period;
			return true;
		}
		t->_next_activation = t->_next_activation + t->_period;
		return false;
	}

private:
	Microsecond _period;
	Microsecond _deadline;
	Microsecond _next_activation;
};

class Alarm
{
public:
	Alarm(const Microsecond & time, Handler * handler, UInt32 times = 1)
	{
		// TCB - to be implemented.
		db<Alarm>(TRC) << "Alarm::Alarm()" << endl;
	}
	static void delay(const Microsecond & time)
	{
		// TCB - to be implemented.
		db<Alarm>(TRC) << "Alarm::delay()" << endl;
	}
	void reset()
	{
		// TCB - to be implemented.
		db<Alarm>(TRC) << "Alarm::reset()" << endl;
	}
};
