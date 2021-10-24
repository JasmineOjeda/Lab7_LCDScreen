/*	Author: Jasmine Ojeda jojed016@ucr.edu
 *	Lab Section: 022
 *	Assignment: Lab 7  Exercise 1
 *	Exercise Description: Incrementer/Decrementer LCD
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtu.be/AkdDKNDAK6Y
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "io.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
    TCCR1B = 0x0B;

    OCR1A = 125;

    TIMSK1 = 0x02;

    TCNT1 = 0;

    _avr_timer_cntcurr = _avr_timer_M;

    SREG |= 0x80;
}

void TimerOff() {
    TCCR1B = 0x00;
}

void TimerISR() {
    TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
    _avr_timer_cntcurr--;
    if (_avr_timer_cntcurr == 0) {
        TimerISR();
	_avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

enum States {START, PB_0, PB_1_to_0, PB_1_to_2, PB_2, WIN} state;

unsigned char pause;
unsigned char win;
unsigned char tmp_A;
unsigned char tmp_D = 0;
unsigned char count = 0;

void Tick() {
    unsigned char tmp_B = PORTB;

    switch (state) {
	case START:
            count = 0;
	    tmp_D = 5;
	    if (!tmp_A) {
	        state = PB_0;
	    }
	    break;
	    
        case PB_0:
	    if (tmp_A && !pause) {
                pause = 1;
		if (tmp_D != 0) {
		    tmp_D--;
		}
            }
	    else if (tmp_A && pause) {
                pause = 0;
		count = 3;
            }
	    if (!pause && (count > 3)) {
                count = 0;
		state = PB_1_to_2;
	    }

	    if (!pause) {
	        count++;
	    }
	    break;

	case PB_1_to_0:
	    if (tmp_A && !pause) {
		tmp_D++;
                pause = 1;
            }
	    else if (tmp_A && pause) {
                pause = 0;
		count = 3;
            }
	    if (!pause && (count > 3)) {
		count = 0;
	        state = PB_0;
	    }

	    if (!pause) {
	        count++;
	    }
	    break;

	case PB_1_to_2:
            if (tmp_A && !pause) {
		tmp_D++;
                pause = 1;
            }
	    else if (tmp_A && pause) {
                pause = 0;
		count = 3;
            }
	    if (!pause && (count > 3)) {
                count = 0;
		state = PB_2;
	    }

	    if (!pause) {
	        count++;
	    }
            break;

	case PB_2:
	    if (tmp_A && !pause) {
		if (tmp_D != 0) {
		    tmp_D--;
		}
                pause = 1;
            }
	    else if (tmp_A && pause) {
                pause = 0;
		count = 3;
            }
	    if (!pause && (count > 3)) {
	        count = 0;
	        state = PB_1_to_0;
	    }

	    if (!pause) {
	        count++;
	    }
	    break;

	case WIN:
	    win = 1;
	    if (tmp_A) {
		pause = 0;
		win = 0;
                state = START;
		tmp_D = 5;
	    }
	    break;

	default:
	    break;
    }

    switch (state) {
	case START:
	    tmp_B = 0x00;
	    break;
        case PB_0:
            tmp_B = 0x01;
            break;
        case PB_1_to_0:
	    tmp_B = 0x02;
            break;
	case PB_1_to_2:
	    tmp_B = 0x02;
	    break;
        case PB_2:
            tmp_B = 0x04;
            break;
	case WIN:
	    tmp_B = 0x07;
	    break;
        default:
            break;
    }
    
    if (tmp_D == 9) {
        state = WIN;
    }

    PORTD = tmp_D;
    PORTB = (~tmp_B) & 0x07;
}

int main() {
    DDRA = 0x00; PORTA = 0xFF;

    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    LCD_init();

    TimerSet(100);
    TimerOn();

    PORTB = 0x01;
    state = START;
    pause = 0;

    while (1) {
	tmp_A = (~PINA) & 0x01;
        Tick();
        
	if (win) {
            LCD_ClearScreen();
            LCD_DisplayString(1, "VICTORY!");
        }
	else {
	    LCD_ClearScreen();
	    LCD_WriteData(PORTD + '0');
        }

	while(!TimerFlag) { };
	TimerFlag = 0;
    }

    return 1;
}
