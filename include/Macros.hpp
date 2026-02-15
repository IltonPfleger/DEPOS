#pragma once

#define ___STR(X) #X
#define __STR(X) ___STR(X)

#define __MACHINE_HEADER <machine/__MACHINE/__MACHINE.hpp>
#define __MACHINE_TRAITS <machine/__MACHINE/Traits.hpp>

#define __CPU_HEADER <architecture/__ARCH/CPU.hpp>
#define __TIMER_HEADER <architecture/__ARCH/Timer.hpp>
#define __IC_HEADER <architecture/__ARCH/IC.hpp>

#define __APPLICATION_TRAITS <app/__APPLICATION/Traits.hpp>
