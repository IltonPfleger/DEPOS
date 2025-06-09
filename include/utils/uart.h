// ---- * UART 16550 * -----
#ifndef UART_H
#define UART_H

#define UART_CLOCK      24000000UL    // 24 MHz, as unsigned long
#define UART_BAUDRATE   115200U
#define UART_BASE       (volatile char *)0x10000000
#define UART_RBR        (UART_BASE + 0)  // Receiver Buffer Register (read only)
#define UART_THR        (UART_BASE + 0)  // Transmitter Holding Register (write only)
#define UART_IER        (UART_BASE + 1)  // Interrupt Enable Register
#define UART_IIR        (UART_BASE + 2)  // Interrupt Identification Register (read only)
#define UART_FCR        (UART_BASE + 2)  // FIFO Control Register (write only)
#define UART_LCR        (UART_BASE + 3)  // Line Control Register
#define UART_MCR        (UART_BASE + 4)  // Modem Control Register
#define UART_LSR        (UART_BASE + 5)  // Line Status Register
#define UART_MSR        (UART_BASE + 6)  // Modem Status Register
#define UART_SCR        (UART_BASE + 7)  // Scratch Register
#define UART_LSB        (UART_BASE + 0)  // Divisor Latch Low (when DLAB=1)
#define UART_MSB        (UART_BASE + 1)  // Divisor Latch High (when DLAB=1)

static void uart_init()
{
    // DISABLE INTERRUPTIONS
    __asm__ volatile("sb %0, 0(%1)" ::"r"(0x00), "r"(UART_IER));

    // SETUP CLOCK DIVISOR
    const short DIVISOR = (UART_CLOCK + UART_BAUDRATE - 1) / (UART_BAUDRATE * 16);
    __asm__ volatile("sb %0, 0(%1)" ::"r"(0x80), "r"(UART_LCR));
    __asm__ volatile("sb %0, 0(%1)" ::"r"(DIVISOR & 0xFF), "r"(UART_LSB));
    __asm__ volatile("sb %0, 0(%1)" ::"r"((DIVISOR >> 8) & 0xFF), "r"(UART_MSB));

    // DISABLE CLOCK DIVISOR ACESS AND SETUP LCR
    __asm__ volatile("sb %0, 0(%1)" ::"r"(0x03), "r"(UART_LCR));
}

static void uart_put(char c)
{
    *UART_BASE = c;
}

#endif
