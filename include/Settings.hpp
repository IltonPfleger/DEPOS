#pragma once
#include <Machine.hpp>

struct Settings {
    struct IO {
        using Device = Machine::IO::UART;
        struct Enable {
            static constexpr const bool ASSERT = false;
        };
    };
    struct Timer {
        struct Enable {
            static constexpr const bool ALARM     = true;
            static constexpr const bool SCHEDULER = true;
        };

        static constexpr const unsigned long FREQUENCY = 100'000;
        static constexpr const unsigned long SCHEDULER = 10'000;
        static constexpr const unsigned long ALARM     = 100;
    };
};
