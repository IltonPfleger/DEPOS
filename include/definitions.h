#ifndef DEFINITIONS_H
#define DEFINITIONS_H

struct machine_s {
    int cpus;
    int xlen;

    struct {
        int order;
        int size;
        struct {
            int order;
            int size;
        } page;
    } memory;
};

constexpr struct machine_s machine = {
    .cpus = 4,
    .xlen = sizeof(void*) * 8,
    .memory =
        {
            .order = 30,
            .size  = (1 << 30),
            .page =
                {
                    .order = 10,
                    .size  = (1 << 10),
                },
        },
};

#endif
