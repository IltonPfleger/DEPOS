#pragma once

#include <Meta.hpp>
#include <Traits.hpp>
#include <Variadic.hpp>
#include <memory/Memory.hpp>

class Console {
    using IO = Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result;
    static void put(char);

  public:
    class Stream {
        friend Console;
        using Modifier = Stream &(*)(Stream &);

      public:
        Stream() : m_base(10), m_panic(0) {}

        Stream &operator<<(Modifier modifier) { return modifier(*this); }

        Stream &operator<<(char c) { return put(c), *this; }

        Stream &operator<<(const char *s) {
            while (s && *s)
                *this << *s++;
            return *this;
        }

        Stream &operator<<(bool b) {
            *this << (b ? '1' : '0');
            return *this;
        }

        template <unsigned Length> Stream &operator<<(const char (&s)[Length]) { return *this << static_cast<const char *>(s); }

        template <Meta::Pointer T> Stream &operator<<(T p) { return *this << reinterpret_cast<void *>(p); }

        template <Meta::Integer T> Stream &operator<<(T x) {
            if constexpr (Meta::Signed<T>::Result) {
                if (m_base == 10 && x < 0) {
                    *this << '-';
                    x = static_cast<unsigned long>(-x);
                }
            }

            if (x == 0) {
                *this << '0';
                return *this;
            }

            char buffer[64];
            int i = 0;
            const char *digits = "0123456789abcdef";

            while (x > 0) {
                buffer[i++] = digits[x % m_base];
                x /= m_base;
            }

            if (m_base == 16) *this << "0x";

            while (i > 0) {
                *this << buffer[--i];
            }

            return *this;
        }

        Stream &operator<<(void *p) {
            unsigned int old = m_base;
            m_base = 16;
            *this << reinterpret_cast<uintptr_t>(p);
            m_base = old;
            return *this;
        }

        bool panicked() {
            unsigned char volatile guard;
            if (m_panic) return true;
            if (m_panic == (reinterpret_cast<unsigned long>(&guard) & ~(Traits<Memory>::StackSize - 1))) return true;
            return false;
        }

      private:
        unsigned int m_base;
        unsigned long m_panic;
    };

  public:
    template <typename T> struct Hex {
        T m_x;

        // FIX 1: The constructor MUST be constexpr for Meta::Signed to work
        constexpr Hex(T x) : m_x(x) {}

        // FIX 2: Conversion operator (needed for the < check in Meta::Signed)
        constexpr operator T() const { return m_x; }

        // FIX 3: Assignment operator (needed for stream manipulation)
        constexpr Hex &operator=(T v) {
            m_x = v;
            return *this;
        }

        // FIX 4: Division assignment (needed for base conversion logic)
        constexpr Hex &operator/=(unsigned int v) {
            m_x /= v;
            return *this;
        }
    };

    static void panic();
    static void println(const char *, ...);
    static void print(char);
    static void print(const char *);
    static void print(unsigned long);

    template <typename T> static void print(Hex<T> x) {
        bool started = false;

        for (int i = (sizeof(x) * 8) - 4; i >= 0; i -= 4) {
            unsigned int current = (x >> i) & 0xF;
            if (current != 0 || started) {
                put(current < 10 ? current + '0' : current - 10 + 'a');
                started = true;
            }
        }
        if (!started) {
            put('0');
        }
    }

    template <Meta::Integer T> static void print(T t) {
        if constexpr (Meta::Signed<T>::Result) {
            if (t < 0) {
                put('-');
                t = -t;
            }
        }
        print(static_cast<unsigned long>(t));
    }

    template <Meta::Pointer T> static void print(T t) {
        Console::print("0x");
        Console::print(Hex(reinterpret_cast<unsigned long>(t)));
    }

    template <typename First, typename Second, typename... Args>
    static void print(First &&first, Second &&second, Args &&...args) {
        Console::print(static_cast<First &&>(first));
        Console::print(", ");
        Console::print(static_cast<Second &&>(second), static_cast<Args &&>(args)...);
    }

    static void init() { new (&cout) Stream; }
    static Stream &endl(Console::Stream &s) {
        s << '\n' << dec;
        return s;
    }
    static Stream &hex(Stream &s) { return s.m_base = 16, s; }
    static Stream &dec(Stream &s) { return s.m_base = 10, s; }
    static Stream &panic(Stream &s) {
        unsigned char volatile MSV;
        if (!s.m_panic) s.m_panic = (reinterpret_cast<unsigned long>(&MSV) & ~(Traits<Memory>::StackSize - 1));
        return s;
    }

  public:
    static inline Stream cout;

  private:
    static inline unsigned int s_panic = 0;
    static inline unsigned int s_column = 0;
};
