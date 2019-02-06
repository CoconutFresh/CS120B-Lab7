#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

enum States {Start, Init, Inc, Dec, Hold, Reset}state;
unsigned char led = 0x00;
unsigned char button = 0x00;
volatile unsigned char TimerFlag = 0x00;
unsigned char count = 0x00;

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

void tick(){
	button = (~PINA & 0x03);
	//Transitions
	switch(state){
		case Start:
		{
			state = Init;
			break;
		}
		
		case Init:
		if(button == 0x01)
		{
			state = Inc;
			break;
		}
		else if(button == 0x02)
		{
			state = Dec;
			break;
		}
		else if(button == 0x03)
		{
			state = Reset;
			break;
		}
		else
		{
			state = Init;
			break;
		}
		
		case Inc:
		state = Hold;
		break;
		
		case Dec:
		state = Hold;
		break;
		
		case Hold:
		if((button == 0x01) || (button == 0x02))
		{
			state = Hold;
			break;
		}
		else if(button == 0x03)
		{
			state = Reset;
			break;
		}
		else
		{
			state = Init;
			break;
		}
		
		case Reset:
		if((button == 0x01) || (button == 0x02))
		{
			state = Reset;
			break;
		}
		else
		{
			state = Init;
			break;
		}
		
		default:
		break;
	}
	switch(state){ //State actions
		case Start:
		count = 0x00;
		break;
		
		case Init:
		break;
		
		case Inc:
		if(count < 0x09)
		{
			count += 0x01;
			break;
		}
		break;
		
		case Dec:
		if(count > 0x00)
		{
			count -= 0x01;
			//PORTB = led;
			break;
		}
		break;
		
		case Hold:
		break;
		
		case Reset:
		count = 0x00;
		//PORTB = led;
		break;
		default:
		break;
	}
	
	LCD_ClearScreen();
	LCD_Cursor(1);
	LCD_WriteData(count + '0');
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	state = Start;
	TimerSet(100);
	TimerOn();
	LCD_init();
	while(1)
	{
		
		tick();
		while (!TimerFlag){}
		TimerFlag = 0x00;
	}
}
