#pragma once

#define __CONCATENATE(X, Y) X##Y
#define ___STRINGIFY(X) #X
#define __STRINGIFY(X) ___STRINGIFY(X)

/* Headers */
#define __HEADER_NAME(X) X.hpp
#define __HEADER_ARCH(X) <architecture/__ARCH/__HEADER_NAME(X)>
#define __HEADER_MACHINE(X) <machine/__MACHINE/__HEADER_NAME(X)>
#define __HEADER_APPLICATION(X) <app/__APPLICATION/__HEADER_NAME(X)>

/* Architecture */
#ifdef __ARCH
#define __CPU_HEADER __HEADER_ARCH(CPU)
#define __MMU_HEADER __HEADER_ARCH(MMU)
#define __TIMER_HEADER __HEADER_ARCH(Timer)
#define __IC_HEADER __HEADER_ARCH(IC)
#define __VIRTUAL_CPU_HEADER __HEADER_ARCH(VirtualCPU)
#endif

/* Machine */
#ifdef __MACHINE
#define __MACHINE_HEADER __HEADER_MACHINE(Machine)
#define __MACHINE_TRAITS_HEADER __HEADER_MACHINE(Traits)
#define __MACHINE_DRIVERS_HEADER __HEADER_MACHINE(drivers)
#endif

/* Application */
#ifdef __APPLICATION
#define __APPLICATION_TRAITS_HEADER __HEADER_APPLICATION(Traits)
#endif
