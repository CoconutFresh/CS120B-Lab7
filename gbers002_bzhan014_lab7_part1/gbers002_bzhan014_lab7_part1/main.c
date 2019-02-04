/*	Name: Glenn Bersabe Email: Gbers002@ucr.edu
	Name: Bohan Zhang Email: bzhan014@ucr.edu
*	Lab Section: 023
*	Assignment: Lab 07  Part 1
*	I acknowledge all content contained herein, excluding template or example
*	code, is my own original work.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

enum states {start, init, wait, s0, s0_wait, s1, s1_wait, reset} state;
unsigned char inc_button = 0;
unsigned char dec_button = 0;
unsigned char count = 0;

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long timer = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long timer_current = 0; // Current internal count of 1ms ticks


void TimerOn() {
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt
	TCNT1=0;
	timer_current = timer;
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}


void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	timer_current--; // Count down to 0 rather than up to TOP
	if (timer_current == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		timer_current = timer;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	timer = M;
	timer_current = timer;
}



void Toggle() {
	static unsigned char i = 0; //counter for holding increase or decrease
	inc_button = (~PINA & 0x01);
	dec_button = (~PINA & 0x02);
	
	switch(state) {
		case start:
			state = init;
			break;
		case init:
			state = wait;
			break;
		case wait:
			if (inc_button && !dec_button) {
				state = s0;
				} else if (dec_button && !inc_button) {
				state = s1;
				} else if (dec_button && inc_button) {
				state = reset;
			}
			break;
		case s0:
			if(inc_button){
				if(dec_button){
					state = reset;
				}
				else{
					state = s0;
					++i;
				}
			}
			else{
				state = s0_wait;
			}
			break;
		case s1:
			if(dec_button){
				if(inc_button){
					state = reset;
				
				}
				else{
					state = s1;
					++i;
				}
			}
			else{
				state = s1_wait;
			}
			break;
		case s0_wait:
			if(dec_button && inc_button){
				state = reset;
			}
			else if (dec_button || inc_button) {
				state = s0_wait;
			} 
			else {
				state = wait;
			}
			break;
		case s1_wait:
			if(dec_button && inc_button){
				state = reset;
			}
			else if (dec_button || inc_button) {
				state = s1_wait;
			}
			else {
				state = wait;
			}
			break;
		case reset:
			if (dec_button || inc_button) {
				state = reset;
			}
			else {
				state = wait;
			}
			break;
	}
	switch(state) {
		case init:
			count = 0x00;
			i = 0;
			break;
		case wait:
			break;
		case s0:
			if (count < 0x09 && (i % 10 == 0 || i == 1) && i != 0)
			count += 1;
			break;
		case s1:
			if (count > 0 && (i % 10 == 0 || i == 1) && i != 0)
			count--;
			break;
		case reset:
			count = 0x00;
			i = 0;
			break;
		case s0_wait:
			i = 0;
			break;
		case s1_wait:
			i = 0;
			break;
	}
	
	LCD_ClearScreen();
	LCD_Cursor(1);
	LCD_WriteData(count + '0');
	
}

int main(void)
{
	state = start;
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	TimerSet(100);
	TimerOn();
	LCD_init ();

	while (1)
	{
		Toggle();
		while (!TimerFlag){}
		TimerFlag = 0;
	}
}