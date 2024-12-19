#include <8051.h>

#define TRUE 1
#define FALSE 0

#define TH_0 231
#define MAX_COUNTER 1152

__xdata unsigned char *CSDS = (__xdata unsigned char *) 0xFF38;
__xdata unsigned char *CSDB = (__xdata unsigned char *) 0xFF30;
__bit __at (0x96) SEG_OFF;

unsigned char SS = 45, MM = 58, HH = 23;

unsigned char display_values[6] = {0};
unsigned char KBD[4] = {0, 0, 0, 0};

#define ENTER 0b000001
#define LEFT 0b100000
#define RIGHT 0b000100
#define DOWN 0b010000

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

static unsigned char idx = 0;

void set_display() {
    display_values[0] = SS % 10;
    display_values[1] = SS / 10;
    display_values[2] = MM % 10;
    display_values[3] = MM / 10;
    display_values[4] = HH % 10;
    display_values[5] = HH / 10;
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

void keyboard_handler() {
    if ((KBD[0] != KBD[1]) && (KBD[0] != KBD[2]) && (KBD[0] != KBD[3])) {
        if (KBD[0] & LEFT) {
            HH = (HH + 1) % 24;
        } else if (KBD[0] & DOWN) {
            MM = (MM + 1);
            if (MM > 59) {
                MM = 0;
                HH = (HH + 1) % 24;
            }
        } else if (KBD[0] & RIGHT) {
			increment_time();
        }
        set_display();
    }

    KBD[3] = KBD[2];
    KBD[2] = KBD[1];
    KBD[1] = KBD[0];
    KBD[0] = 0;
}

void refresh_display() {
    static unsigned char mask = 1;
    unsigned char value;

    value = display_values[idx];
    SEG_OFF = TRUE;
    *CSDB = mask;
    *CSDS = patterns[value];
	SEG_OFF = FALSE;

    if (P3_5) {
        KBD[0] |= mask;
    }

    idx = (idx + 1) % 6;
    mask = mask << 1;
	if (mask == 32) {
        if (KBD[0] != 0) {
            keyboard_handler();
        } else {
         	KBD[0] = 0;
			KBD[1] = 0;
			KBD[2] = 0;
			KBD[3] = 0;
        }
    }
    else if (mask > 32) {
        mask = 1;
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
    set_display();

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
        set_display();
        counter = MAX_COUNTER;
    }
}
