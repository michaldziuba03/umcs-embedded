#include <8051.h>

#define TRUE 1
#define FALSE 0

#define TH_0 231
#define MAX_COUNTER 1152

__xdata unsigned char *CSDS = (__xdata unsigned char *) 0xFF38;
__xdata unsigned char *CSDB = (__xdata unsigned char *) 0xFF30;
__bit __at (0x96) SEG_OFF;

unsigned char SS = 45, MM = 36, HH = 17;

const unsigned char patterns[] = {
    0b00111111, 0b00000110,
    0b01011011, 0b01001111,
    0b01100110, 0b01101101,
    0b01111101, 0b00000111,
    0b01111111, 0b01101111,
};

void t0_int(void) __interrupt(1) {
    F0 = TRUE;
    TH0 = TH0 | TH_0;
}

void refresh_display() {
    static unsigned char digit = 1;
    unsigned char value = 0;

    switch (digit) {
        case 1:
            value = SS % 10;
            break;
        case 2:
            value = SS / 10;
            break;
        case 4:
            value = MM % 10;
            break;
        case 8:
            value = MM / 10;
            break;
        case 16:
            value = HH % 10;
            break;
        case 32:
            value = HH / 10;
            break;
        default:
            value = 0;
            break;
    }

    SEG_OFF = TRUE;
    *CSDB = digit;
    *CSDS = patterns[value];
    SEG_OFF = FALSE;

    digit = digit << 1;
    if (digit > 32) {
        digit = 1;
    }
}

void increment_time() {
    SS++;
    if (SS >= 60) {
        SS = 0;
        MM++;
        if (MM >= 60) {
            MM = 0;
            HH++;
            if (HH >= 24) {
                HH = 0;
            }
        }
    }
}

void main() {
    int counter;
    TMOD = 0b01110000;
    TH0 = TH_0;
    ET0 = TRUE;
    EA = TRUE;
    TR0 = TRUE;
    F0 = FALSE;
    P1_7 = 0;
    counter = MAX_COUNTER;

    for (;;) {
        if (!F0) {
           continue;
        }
        F0 = FALSE;
        refresh_display();
        counter--;
        if (counter)
           continue;
        P1_7 = !P1_7;
        increment_time();
        counter = MAX_COUNTER;
    }
}
