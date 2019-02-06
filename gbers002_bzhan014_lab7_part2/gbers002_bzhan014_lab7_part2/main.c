/*	Name: Glenn Bersabe Email: Gbers002@ucr.edu
	Name: Bohan Zhang Email: bzhan014@ucr.edu
*	Lab Section: 023
*	Assignment: Lab 07  Part 2
*	I acknowledge all content contained herein, excluding template or example
*	code, is my own original work.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

enum States{start, led1, led2, led3}state;
volatile unsigned char timerFlag = 0x00;
unsigned char LED = 0x00;
unsigned char button = 0x00;
unsigned char count;

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
		if (timerFlag < 0x04) {
			timerFlag++;
		}
		else if (timerFlag == 0x04) {
			timerFlag == 0x00;
		}
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


void tick() {
	switch(state) { //Transitions
		case start:
			state = led1;
			break;
		case led1:
			if (timerFlag == 0x01) {
				state = led2;
			}
			else {
				state = led1;
			}
			break;
		case led2:
			if (timerFlag == 0x02) {
				state = led3;
			}
			else if (timerFlag == 0x04) {
				state = led1;
			}
			else {
				state = led2;
			}
		case led3:
			if (timerFlag == 0x03) {
				state = led2;
			}
			else {
				state = led3;
			}
			break;
	}
	switch (state) { //ACTIONS
		case start:
			printf("BROKEN");
			break;
		case led1:
			LED = 0x01;
			if (button) {
				count--;
			}
			break;
		case led2:
			LED = 0x02;
			if (button) {
				count++;
			}
			break;
		case led3:
			LED = 0x04;
			if (button) {
				count--;
			}
			break;
		default:
			printf("BROKEN");
			break;
	}
	LCD_ClearScreen();
	LCD_Cursor(1);
	LCD_WriteData(count + '0');
	PORTB = LED; //OUTPUTS LED TO PHYSICAL LED
}


int main(void)
{	
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	
	TimerSet(300);
	TimerOn();
	
	state = start;
	count = 0x05;
	LCD_init();
	while (1)
	{
		button = (~PINA & 0x03);
		tick();
	}
}