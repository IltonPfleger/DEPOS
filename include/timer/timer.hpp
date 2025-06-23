#ifndef TIMER_HPP
#define TIMER_HPP

#include <definitions.hpp>
#include <cpu.hpp>

struct Timer {
    static constexpr uintptr_t INTERVAL = 100000;
    static volatile inline char* BaseAddr() { return reinterpret_cast<volatile char*>(0x02000000); }

    static volatile inline uintptr_t& MTIME() {
        return *reinterpret_cast<volatile uintptr_t*>(BaseAddr() + 0xBFF8);
    }
    static volatile inline uintptr_t& MTIMECMP() {
        return *reinterpret_cast<volatile uintptr_t*>(BaseAddr() + 0x4000);
    }

	static void init() {
		//set();
		//CPU::enable_timer_interrupts();
	}

    static void set() {
        uintptr_t now = MTIME();
        MTIMECMP()    = now + INTERVAL;
    }
};

#endif
