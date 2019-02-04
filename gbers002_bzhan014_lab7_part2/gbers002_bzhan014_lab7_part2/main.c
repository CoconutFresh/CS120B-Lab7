/*	Name: Glenn Bersabe Email: Gbers002@ucr.edu
	Name: Bohan Zhang Email: bzhan014@ucr.edu
*	Lab Section: 023
*	Assignment: Lab 07  Part 1
*	I acknowledge all content contained herein, excluding template or example
*	code, is my own original work.
*/

#increaselude <avr/io.h>
#increaselude <avr/interrupt.h>
#increaselude "io.c"

enum States{start, init, LED1, led2, led3, wait, wait2, increase, decrease, victory} state;

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long timer = 1; // start count from here, down to 0. Default 1 ms.
unsigned long timer_current = 0; // Current internal count of 1ms ticks
unsigned char button = 0;
unsigned char score = 0;
unsigned char i = 0;
const unsigned char* msg = "YOU WIN!", '\0';
void TimerOn() {
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//initialize avr counter
	TCNT1=0;

	timer_current = timer;
	// TimerISR will be called every timer_current milliseconds

	//Enable global interrupts
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

void Tick(){
	button = (~PINA & 0x01);
	
	switch (state)
	{
		case start: 
			state = init;
			break;
		case init: 
			state = LED1;
			break;
		case LED1: 
			state = button ? decrease : led2;
			break;
		case led2: 
			state =  button ? increase : led3;
			break;
		case led3: 
			state =  button ? decrease : LED1;
			break;
		case wait:
			if(score == 9) {
				state = victory;
			}
			else {
				state = button ? wait : wait2;
			}
			break;
		case wait2: 
			state = button ? LED1 : wait2;
			break;
		case increase: 
			state = wait;
			break;
		case decrease:
			 state = wait;
			break;
		case victory: 
			state = i < 3 ? victory :start;// stay in victory state for 3 transitions
			default: state = start;
		break;
	}
	
	switch (state)
	{
		case start:
			break;
		case init:
			PORTB = 0x00;
			score = 5;
			break;
		case LED1: 
			PORTB = 0x01;
			break;
		case led2: 
			PORTB = 0x02;
			break;
		case led3: 
			PORTB = 0x04;
			break;
		case wait: 
			LCD_ClearScreen();
			LCD_Cursor(1);
			LCD_WriteData(score + '0');
			break;
		case increase: 
			++score;
			break;
		case decrease: 
			if(score > 0) {
				--score;
			}
			break;
		case victory:   
			LCD_ClearScreen();
			LCD_DisplayString(1, msg);
			++i;
		break;
	}
}

int main(void)
{
	state = start;
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	TimerSet(300);
	TimerOn();
	
	
	while (1)
	{
		Tick();
		while (!TimerFlag){}
		TimerFlag = 0;
		
	}
}