export module Settings;
import Machine;

export struct Settings {
    struct IO {
        using Device = Machine::IO::UART;
    };
    struct Timer {
        struct Enable {
            static constexpr const bool ALARM     = true;
            static constexpr const bool SCHEDULER = false;
        };

        static constexpr const unsigned long FREQUENCY = 100;
        static constexpr const unsigned long SCHEDULER = 1'000;
        static constexpr const unsigned long ALARM     = 10;
    };
};
