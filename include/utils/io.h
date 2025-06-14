#ifndef PRINT_H
#define PRINT_H

#define IO UART
#define IO_EXPAND(X, Y) X##Y
#define IO_INIT IO_EXPAND(IO, _init)()
#define IO_PUT(X) IO_EXPAND(IO, _put)(X)

void kprint(const char* format, ...);

#endif
