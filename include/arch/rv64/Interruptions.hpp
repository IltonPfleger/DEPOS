struct Interruptions {
    static void disable() { csrc<KernelMode::STATUS>(KernelMode::IRQE); }
    static void enable() { csrs<KernelMode::STATUS>(KernelMode::IRQE); }

    static void on() { enable(); }
    static bool off() {
        unsigned long status = csrr<KernelMode::STATUS>();
        disable();
        return (status & KernelMode::IRQE) != 0;
    }
};
