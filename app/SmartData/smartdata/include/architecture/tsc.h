#pragma once

// EPOS IA32 Time-Stamp Counter Mediator Declarations

#include <architecture/cpu.h>
#include <architecture/tsc.h>
#include <system/types.h>
#include <chrono>
#include <unistd.h>
#include <sys/time.h>

class TSC
{
public:
    typedef UInt64 Time_Stamp;

public:
    TSC() {}

    static Hertz frequency() { 
        static Hertz frequency = 0;
        if (frequency == 0) {
            TSC::Time_Stamp t0 = TSC::time_stamp();
            usleep(1000000);
            TSC::Time_Stamp t1 = TSC::time_stamp();
            frequency = t1 - t0;
        }
        return frequency; 
    }
    static PPB accuracy() { return 50; }

    static Time_Stamp time_stamp() {
        if (Traits<Build>::ARCHITECTURE == Traits<Build>::ARMv8) {
            struct timeval time;
            gettimeofday(&time, nullptr);
            return (time.tv_sec * 1000000 + time.tv_usec);
        } else {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::
                  now().time_since_epoch()).count();
        }
        // Time_Stamp ts;
        // __asm__ __volatile__ ("rdtsc" : "=A" (ts) : ); // must be volatile!
        // return ts;
    }

    static void time_stamp(const Time_Stamp & ts) {
       // CPU::wrmsr(CPU::MSR_TSC, ts);
    }
};
