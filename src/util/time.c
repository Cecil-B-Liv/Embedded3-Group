#define CPU_FREQ_MHZ 48  // 1 GHz = 1000 MHz

void delay_ms(unsigned int ms) {
    volatile unsigned int count;
    while (ms--) {
        count = CPU_FREQ_MHZ * 1000;  // Approx 1 million cycles per ms
        while (count--) {
            __asm__ volatile("nop");
        }
    }
}
