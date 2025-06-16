#ifndef IO_H
#define IO_H

struct io_interface_s {
    void (*put)(char);
    char (*get)(void);
};

void io_init(struct io_interface_s);
void io_out(const char* format, ...);

#endif
