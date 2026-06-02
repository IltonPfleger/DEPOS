#pragma once

#define __CONCATENATE(X, Y) X##Y
#define ___STRINGIFY(X) #X
#define __STRINGIFY(X) ___STRINGIFY(X)

///* Headers */
#define __HEADER_NAME(X) X.hpp
// #define __HEADER_MACHINE(X) <machine/__MACHINE/__HEADER_NAME(X)>
#define __HEADER_APPLICATION(X) <app/__APPLICATION/__HEADER_NAME(X)>
//
///* Machine */
// #ifdef __MACHINE
// #define __MACHINE_HEADER __HEADER_MACHINE(Machine)
// #define __MACHINE_TRAITS_HEADER __HEADER_MACHINE(Traits)
// #define __MACHINE_DRIVERS_HEADER __HEADER_MACHINE(drivers)
// #endif

/* Application */
#ifdef __APPLICATION
#define __APPLICATION_TRAITS_HEADER __HEADER_APPLICATION(Traits)
#endif
