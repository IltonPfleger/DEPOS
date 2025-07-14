#pragma once
#include <CPU.hpp>
#include <Thread.hpp>

 struct RawSemaphore {
     Thread::List waiting;
     int value;

     RawSemaphore(int value) : value(value) {}

     void p() {
         if (CPU::Atomic::fdec(&value) < 0) {
             Thread::sleep(&waiting);
         }
     }

     void v() {
         if (CPU::Atomic::fadd(&value) <= 0) {
             Thread::wakeup(&waiting);
         }
     }
 };

 struct Semaphore : private RawSemaphore {
     Semaphore(int value) : RawSemaphore(value) {}

     void p() {
         CPU::Interrupt::disable();
         RawSemaphore::p();
         CPU::Interrupt::enable();
     }

     void v() {
         CPU::Interrupt::disable();
         RawSemaphore::v();
         CPU::Interrupt::enable();
     }
 };
