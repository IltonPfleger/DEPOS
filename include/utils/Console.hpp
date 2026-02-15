#pragma once

#include <Meta.hpp>
#include <Traits.hpp>
#include <Variadic.hpp>

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
        explicit Hex(T x) : m_x(x) {}
        operator T() const { return m_x; }

      private:
        T m_x;
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

  public:
    static inline Stream cout;

  private:
    static inline unsigned int s_panic = 0;
    static inline unsigned int s_column = 0;
};
