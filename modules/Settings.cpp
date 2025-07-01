export module Settings;

export struct Settings {
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
