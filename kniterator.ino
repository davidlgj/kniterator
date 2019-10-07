
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>

// OLED display code https://bitbucket.org/tinusaur/ssd1306xled
#include "ssd1306xled.h"

#define text(x, y, s) ssd1306_string_font8x16xy(x, y, s)
#define number(x, y, n) ssd1306_numdecp_font8x16(x, y, n)

#define ADD_BTN 3
#define DEC_BTN 4

#define SLEEP_TRESHOLD 5000

uint16_t counter = 0;
unsigned long last_time = 0;
unsigned long current_time = 0;
int btn;

ISR(PCINT0_vect) {
  // do something interesting here
}

void setup(){
  delay(40); // Give the ssd1306 a chance to startup
  ssd1306_init();

  pinMode(ADD_BTN, INPUT_PULLUP);
  pinMode(DEC_BTN, INPUT_PULLUP);

  // We setup pin change interrupts for wake up on both buttons. But we don't enable it yet.
  PCMSK  |= bit(PCINT3);
  PCMSK  |= bit(PCINT4);

  ssd1306_clear();
  text(0, 0, "Reticulating");
  text(0, 2, "splines...");
  delay(2000);
  resetTime();

  // Print initial 0
  printCounter();
}

void loop() {
  current_time = millis();
  if (current_time - last_time > SLEEP_TRESHOLD) {
    sleep(); // Go to sleep.
    delay(50); // Delay a bit to debounce the push
    // We don't like inc/dec on the actual push button that woke us, so let's busy loop wait for
    // them to be released.
    while (digitalRead(ADD_BTN) == LOW || digitalRead(DEC_BTN) == LOW) {
      delay(1);
    }
    delay(100); // Delay a bit to debounce the push

    // Reset time again.
    resetTime();
  }

  // Check for ADD btn
  btn = digitalRead(ADD_BTN);
  if (btn == LOW) {
    counter++;
    printCounter();
    resetTime(); // Keep the screen on a bit longer.
    delay(150); // Simple debounce by blocking wait
  }

  // Check for decrease
  btn = digitalRead(DEC_BTN);
  if (btn == LOW) {
    if (counter > 0) {
      counter--;
    }
    printCounter();
    resetTime(); // Keep the screen on a bit longer.
    delay(150); // Simple debounce by blocking wait
  }
}

void resetTime() {
  last_time = millis();
  current_time = last_time;
}

void printCounter() {
  ssd1306_clear();
  text(0, 2, "Rounds knitted:");
  number(30, 4, counter);
}

void sleep() {
  // We turn off screen to save power.
  ssd1306_off();

  // First we enable the pin change interrupt that will wake us up
  GIFR   |= bit(PCIF); // clear any outstanding interrupts
  GIMSK  |= bit(PCIE); // enable pin change interrupts

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  ADCSRA = 0; // turn off ADC
  power_all_disable (); // power off ADC, Timer 0 and 1, serial interface

  sleep_enable();
  sleep_cpu(); // Sleep!
  sleep_disable();

  GIMSK  &= ~bit(PCIE); // disable pin change interrupts
  power_all_enable(); // power everything back on
  ssd1306_on();
}
