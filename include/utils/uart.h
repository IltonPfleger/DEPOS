#ifndef UART_H
#define UART_H

#define UART_CLOCK      24000000UL
#define UART_BAUDRATE   115200U
#define UART0           ((volatile uint8_t *)0x10000000)
#define UART_RBR        (UART0 + 0)
#define UART_THR        (UART0 + 0)
#define UART_IER        (UART0 + 1)
#define UART_IIR        (UART0 + 2)
#define UART_FCR        (UART0 + 2)
#define UART_LCR        (UART0 + 3)
#define UART_MCR        (UART0 + 4)
#define UART_LSR        (UART0 + 5)
#define UART_MSR        (UART0 + 6)
#define UART_SCR        (UART0 + 7)
#define UART_LSB        (UART0 + 0)
#define UART_MSB        (UART0 + 1)

#define LSR_TX_FIFO_EMPTY (1 << 5)

static void uart_init(void) {
    // disable interrupts.
    *UART_IER = 0x00;

    // enable divisor latch acess
    *UART_LCR         = (1 << 7);
    const int divisor = (UART_CLOCK + UART_BAUDRATE * 8) / (UART_BAUDRATE * 16);
    *UART_LSB         = divisor & 0xFF;
    *UART_MSB         = (divisor >> 8) & 0xFF;

    // 8 bits, no parity, 1 stop
    *UART_LCR = 0x03;

    // FIFO + reset TX/RX
    *UART_FCR = 0x07;

    // enable interrupts.
    *UART_IER = 0x03;
}

static void uart_put(char c) {
    while (!(*UART_LSR & LSR_TX_FIFO_EMPTY));
    *UART_THR = c;
}

#endif
