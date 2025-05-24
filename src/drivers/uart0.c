#if 1

#include "uart0.h"
#include "mbox.h"

uart_mode_t currentMode;

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init(uart_mode_t mode, int baudrate) {
    unsigned int r;

    /* Turn off UART0 */
    UART0_CR = 0x0;

    /* Set GPIO14 (TXD0) and GPIO15 (RXD0) to ALT0 */
    r = GPFSEL1;
    r &= ~((7 << 12) | (7 << 15));      // Clear FSEL14, FSEL15
    r |= (0b100 << 12) | (0b100 << 15); // ALT0
    GPFSEL1 = r;

    /* If handshake mode, set GPIO16 (CTS0) and GPIO17 (RTS0) to ALT3 */
    if (mode == UART0_MODE_HANDSHAKE) {
        r = GPFSEL1;
        r &= ~((7 << 18) | (7 << 21));      // Clear FSEL16, FSEL17
        r |= (0b011 << 18) | (0b011 << 21); // ALT3
        GPFSEL1 = r;
    }

#ifdef RPI3
    GPPUD = 0;
    r = 150;
    while (r--)
        asm volatile("nop");
    GPPUDCLK0 = (1 << 14) | (1 << 15);
    if (mode == UART0_MODE_HANDSHAKE) {
        GPPUDCLK0 |= (1 << 16) | (1 << 17);
    }
    r = 150;
    while (r--)
        asm volatile("nop");
    GPPUDCLK0 = 0;
#else
    r = GPIO_PUP_PDN_CNTRL_REG0;
    r &= ~((3 << 28) | (3 << 30)); // GPIO14, GPIO15
    if (mode == UART0_MODE_HANDSHAKE)
    {
        r &= ~((3 << 0) | (3 << 2)); // GPIO16, GPIO17
    }
    GPIO_PUP_PDN_CNTRL_REG0 = r;
#endif

    UART0_IMSC = 0;
    UART0_ICR = 0x7FF;

    UART0_IBRD = 26;
    UART0_FBRD = 3;

    int *BR = calculate_IBRD(baudrate);
    UART0_IBRD = BR[0]; // Integer part
    UART0_FBRD = BR[1]; // Fractional part


    UART0_LCRH = UART0_LCRH_FEN | UART0_LCRH_WLEN_8BIT;

    UART0_CR = 0x301;
    if (mode == UART0_MODE_HANDSHAKE) {
        UART0_CR |= UART0_CR_CTSEN | UART0_CR_RTSEN;
    }

    currentMode = mode; // Save current mode globally if needed
}

// set baudrate
void uart_setBaudrate(int baudrate) {
    /* Turn off UART0 */
    UART0_CR = 0x0;

    UART0_IMSC = 0;
    UART0_ICR = 0x7FF;

    // UART0_IBRD = 26;
    // UART0_FBRD = 3;
    int *BR = calculate_IBRD(baudrate);
    UART0_IBRD = BR[0]; // Integer part
    UART0_FBRD = BR[1]; // Fractional part

    UART0_LCRH = UART0_LCRH_FEN | UART0_LCRH_WLEN_8BIT;

    UART0_CR = 0x301;
    if (currentMode == UART0_MODE_HANDSHAKE) {
        UART0_CR |= UART0_CR_CTSEN | UART0_CR_RTSEN;
    }
}

/**
 * Send a character
 */
void uart_sendc(char c) {

    /* Check Flags Register */
    /* And wait until transmitter is not full */
    do {
        asm volatile("nop");
    } while (UART0_FR & UART0_FR_TXFF);

    /* Write our data byte out to the data register */
    UART0_DR = c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char c = 0;

    /* Check Flags Register */
    /* Wait until Receiver is not empty
     * (at least one byte data in receive fifo)*/
    do {
        asm volatile("nop");
    } while (UART0_FR & UART0_FR_RXFE);

    /* read it and return */
    c = (unsigned char) (UART0_DR);

    /* convert carriage return to newline */
    return (c == '\r' ? '\n' : c);
}

/**
 * Display a string
 */
void uart_puts(char *s) {
    while (*s) {
        /* convert newline to carriage return + newline */
        if (*s == '\n')
            uart_sendc('\r');
        uart_sendc(*s++);
    }
}

/**
 * Display a value in hexadecimal format
 */
void uart_hex(unsigned int num) {
    uart_puts("0x");
    for (int pos = 28; pos >= 0; pos = pos - 4) {

        // Get highest 4-bit nibble
        char digit = (num >> pos) & 0xF;

        /* Convert to ASCII code */
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        digit += (digit > 9) ? (-10 + 'A') : '0';
        uart_sendc(digit);
    }
}

/*
**
* Display a value in decimal format
*/
void uart_dec(int num) {
    // A string to store the digit characters
    char str[33] = {0};

    // Calculate the number of digits
    int len = 1;
    int temp = num;
    while (temp >= 10) {
        len++;
        temp = temp / 10;
    }

    // Store into the string and print out
    for (int i = 0; i < len; i++) {
        int digit = num % 10; // get last digit
        num = num / 10;       // remove last digit from the number
        str[len - (i + 1)] = digit + '0';
    }
    str[len] = '\0';

    uart_puts(str);
}

// int *caculate_IBRD(int baudrate)
// {

//     static int BR[2] = {0};

//     unsigned int r;
//     unsigned int UART_CLOCK = 4000000; // 4MHz
//     float divider = (float)UART_CLOCK / (16 * baudrate);

//     r = (unsigned int)divider; // Integer part of the divider
//     BR[0] = r;                 // Integer part

//     float fractional = divider - r;
//     unsigned int n = (unsigned int)(fractional * 64 + 0.5);

//     BR[1] = n; // Fractional part

//     return BR;
// }

int *calculate_IBRD(int baudrate) {
    static int BR[2] = {0};

    unsigned int UART_CLOCK = 48000000; // Correct UART clock for Pi 3
    float divider = (float) UART_CLOCK / (16 * baudrate);

    unsigned int integer = (unsigned int) divider;
    float fractional = divider - integer;
    unsigned int frac = (unsigned int) (fractional * 64 + 0.5f);

    BR[0] = integer; // IBRD
    BR[1] = frac;    // FBRD

    return BR;
}


void uart_mac_formatter(unsigned int num) {
    for (int pos = 4; pos >= 0; pos = pos - 4) {
        // Get highest 4-bit nibble
        char digit = (num >> pos) & 0xF;

        // Convert to ASCII code
        digit += (digit > 9) ? (-10 + 'A') : '0';

        uart_sendc(digit); // Send the character
    }
}

char uart_get_escape_sequence() {
    static uint8_t escape_seq = 0;
    char c = uart_getc();

    if (escape_seq == 0 && c == 0x1B) {
        escape_seq = 1;
        return 0; // signal "in progress"
    } else if (escape_seq == 1 && c == '[') {
        escape_seq = 2;
        return 0;
    } else if (escape_seq == 2) {
        escape_seq = 0;
        switch (c) {
            case 'A':return 0x81;  // Up arrow
            case 'B':return 0x82;  // Down arrow
            case 'C':return 0x83;  // Right arrow
            case 'D':return 0x84;  // Left arrow
            default:return 0;
        }
    } else if ((unsigned char) c == 0x00 || (unsigned char) c == 0xE0) {
        char code = uart_getc();
        switch ((unsigned char) code) {
            case 0x48: return 0x81; // Up arrow
            case 0x50: return 0x82; // Down arrow
            case 0x4D: return 0x83; // Right arrow
            case 0x4B: return 0x84; // Left arrow
        }
        return 0;

    } else {
        escape_seq = 0;
        return c;
    }
}

#endif