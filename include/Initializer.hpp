#pragma once

void* operator new(unsigned long, void*);

template <typename Derived>
class Initializer {
   public:
    template <typename... Args>
    static void init(Args&&... args) {
        static char __system[sizeof(Derived)];
        _SYSTEM = new (reinterpret_cast<void*>(__system)) Derived(static_cast<Args&&>(args)...);
    }

    static Derived* SYSTEM() { return _SYSTEM; }

   private:
    static inline Derived* _SYSTEM;
};
