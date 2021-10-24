/*	Author: Jasmine Ojeda jojed016@ucr.edu
 *	Lab Section: 022
 *	Assignment: Lab 7  Exercise 1
 *	Exercise Description: Incrementer/Decrementer LCD
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtu.be/2XLN1Oy_S3c
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

enum States {START, WAIT, INCREMENT, DECREMENT, INC_PRESS, DEC_PRESS, RES_PRESS, RESET} state;

unsigned char tmp_D;
unsigned char tmp_A;
unsigned char count = 0;

void Tick() {
    /* State transitions */
    switch(state) {
        case START:
            tmp_D = 0x07;
            state = WAIT;
            break;

        case WAIT:
            if (tmp_A == 0x00) {
                state = WAIT;
            }
            else if (tmp_A == 0x01) {
                state = INCREMENT;
            }
            else if (tmp_A == 0x02) {
                state = DECREMENT;
            }
            else if (tmp_A == 0x03) {
                state = RESET;
            }
            else {
                state = WAIT;
            }
            break;

        case INCREMENT:
            state = INC_PRESS;
            break;

        case DECREMENT:
            state = DEC_PRESS;
            break;

        case RESET:
            state = RES_PRESS;
            break;

        case INC_PRESS:
            if (tmp_A == 0x00) {
                state = WAIT;
            }
	    else if (tmp_A == 0x02) {
                state = DECREMENT;
	    }
	    else if (tmp_A == 0x03) {
                state = RESET;
            }
            else {
		if (count == 10) {
		    count = 0;
                    state = INCREMENT;
		}
		else {
                    count++;
	        }
            }
            break;

	case DEC_PRESS:
            if (tmp_A == 0x00) {
                state = WAIT;
            }
            else if (tmp_A == 0x01) {
                state = INCREMENT;
            }
            else if (tmp_A == 0x03) {
                state = RESET;
            }
            else {
		if (count == 10) {
	            count = 0;
                    state = DECREMENT;
		}
		else {
                    count++;
		}
            }
            break;

	case RES_PRESS:
            if (tmp_A == 0x00) {
                state = WAIT;
            }
	    else if (tmp_A == 0x01) {
                state = INCREMENT;
            }
            else if (tmp_A == 0x02) {
                state = DECREMENT;
            }
            else if (tmp_A == 0x03) {
                state = RESET;
            }
            else {
                state = RES_PRESS;
            }
            break;

        default:
            break;
    }

    /* State actions */
    switch(state) {
        case START:
            tmp_D = 0x07;
            break;

        case INCREMENT:
            if (tmp_D < 9) {
                tmp_D++;
            }
            break;

        case DECREMENT:
            if (tmp_D > 0) {
                tmp_D--;
            }
            break;

        case RESET:
            tmp_D = 0x00;
            break;
        case INC_PRESS: break;
	case DEC_PRESS: break;
	case RES_PRESS: break;
        case WAIT: break;
	default: break;
    }

    PORTD = tmp_D;
}

int main() {
    DDRA = 0x00; PORTA = 0xFF;

    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    LCD_init();

    TimerSet(100);
    TimerOn();


    state = START;

    while (1) {
	tmp_A = (~PINA) & 0x03;
        Tick();

	LCD_ClearScreen();
	LCD_WriteData(PORTD + '0');

	while(!TimerFlag) { };
	TimerFlag = 0;
    }

    return 1;
}
