struct Interruptions {
    static void disable() { csrc<KernelMode::STATUS>(KernelMode::IRQE); }
    static void enable() { csrs<KernelMode::STATUS>(KernelMode::IRQE); }

    static void on() { enable(); }
    static bool off() {
        Register status = csrr<KernelMode::STATUS>();
        disable();
        return (status & KernelMode::IRQE) != 0;
    }
};
