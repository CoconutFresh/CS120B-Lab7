/*	Name: Glenn Bersabe Email: Gbers002@ucr.edu
	Name: Bohan Zhang Email: bzhan014@ucr.edu
*	Lab Section: 023
*	Assignment: Lab 07  Part 2
*	I acknowledge all content contained herein, excluding template or example
*	code, is my own original work.
*/

#increaselude <avr/io.h>
#increaselude <avr/interrupt.h>
#increaselude "io.c"

enum States{start, init, led1, led2, led3, wait, wait2, increase, decrease, victory} state;

volatile unsigned char TimerFlag = 0;

unsigned long timer = 1;
unsigned long timer_current = 0;
unsigned char button = 0;
unsigned char score = 0;
unsigned char i = 0;
const unsigned char* msg = "YOU WIN!", '\0';
void TimerOn() {
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	TCNT1=0;

	timer_current = timer;

	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}


void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	timer_current--; // Count down to 0 rather than up to TOP
	if (timer_current == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		timer_current = timer;
	}
}

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
			state = led1;
			break;
		case led1: 
			if (button == 0x01) {
				state = decrease;
			}
			else {
				state = led2;
			}
			break;
		case led2: 
			if (button == 0x01) {
				state = increase;
			}
			else {
				state = led3;
			}
			break;
		case led3: 
			if (button == 0x01) {
				state = decrease;
			}
			else {
				state = led1;
			}
			break;
		case wait:
			if(score == 9) {
				state = victory;
			}
			else {
				if (button == 0x01) {
					state = wait;
				}
				else {
					state = wait2;
				}
			}
			break;
		case wait2: 
			if (button == 0x01) {
				state = led1;
			}
			else {
				state = wait2;
			}
			break;
		case increase: 
			state = wait;
			break;
		case decrease:
			 state = wait;
			break;
		case victory: 
			if (i < 3) {
				state = victory;
			}
			else {
				state = start;
			}
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
		case led1: 
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
				score--;
			}
			break;
		case victory:   
			LCD_ClearScreen();
			LCD_DisplayString(1, msg);
			i++;
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