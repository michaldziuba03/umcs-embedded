/*
 * Author: Michał Dziuba (mail@michaldziuba.dev)
 * All rights reserved.
 * This code is proprietary and may not be used, copied, modified, or distributed
 * without explicit permission from the author.
 */

#include <8051.h>

#define TRUE 1
#define FALSE 0

#define TH_0 231
#define MAX_COUNTER 1152

__xdata unsigned char *CSDS = (__xdata unsigned char *)0xFF38;
__xdata unsigned char *CSDB = (__xdata unsigned char *)0xFF30;
__bit __at(0x96) SEG_OFF;

unsigned char SS = 45, MM = 36, HH = 17;

unsigned char display_values[6] = {0};

const unsigned char patterns[] = {
    0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
    0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111,
};

void t0_int(void) __interrupt(1) {
  F0 = TRUE;
  TH0 = TH0 | TH_0;
}

void refresh_display() {
  static unsigned char digit = 1;
  static unsigned char idx = 0;
  unsigned char value;

  value = display_values[idx];
  SEG_OFF = TRUE;
  *CSDB = digit;
  *CSDS = patterns[value];
  SEG_OFF = FALSE;

  idx = (idx + 1) % 6;
  digit = digit << 1;
  if (digit > 32) {
    digit = 1;
  }
}

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
