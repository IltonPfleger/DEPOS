#pragma once
#include <Machine.hpp>
#include <Meta.hpp>

constexpr char HEX[] = "0123456789ABCDEF";

struct Console {
    static inline void init() { Machine::IO::init(); }
    static void put(char c) { Machine::IO::put(c); }

    class Stream {
        using Manipulator = Stream& (*)(Stream&);

       public:
        inline Stream& operator<<(Manipulator m) { return m(*this); }

        inline Stream& operator<<(char c) {
            Console::put(c);
            return *this;
        }

        inline Stream& operator<<(const char* str) {
            while (*str) Console::put(*str++);
            return *this;
        }

        template <Meta::INTEGRAL T>
        inline Stream& operator<<(T value) {
            char buffer[32];
            int position = 0;

            if constexpr (Meta::SIGNED<T>::Result) {
                if (value < 0) {
                    Console::put('-');
                    value = -value;
                }
            }

            do {
                buffer[position++] = '0' + (value % 10);
                value /= 10;
            } while (value > 0);

            while (position--) Console::put(buffer[position]);
            return *this;
        }

        inline Stream& operator<<(void* pointer) {
            auto addr = reinterpret_cast<unsigned long>(pointer);
            Console::put('0');
            Console::put('x');
            for (int i = (sizeof(void*) * 2) - 1; i >= 0; i--) {
                Console::put(HEX[(addr >> (i * 4)) & 0xF]);
            }
            return *this;
        }

        static Stream& endl(Stream& s) {
            put('\n');
            return s;
        }
    };

   public:
    static inline Stream out;
};
