/*
 * @file avr_timer.h
 * @brief TCA driver for the ATtiny1617. Sets up TCA as a 1ms timer with overflow interrupt.
 *
 * To use with Modbus Client:
 *     Construct a Timer object and pass into the attiny1617_ModbusClient constructor.
 *
 * Created: 2022-11-16 3:42:39 PM
 *  Author: sjeffery
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>

class Timer {

	TCA_t * timer_base;	// Pointer to TCA peripheral
	
	uint32_t ticks = 0; // 1ms overflows
	
	public:
	
	/**
	 * @brief Timer constructor.
	 * @param _usart_base	Pointer to the USART peripheral
	 */
	Timer(
		TCA_t * _timer_base
	) :
		timer_base(_timer_base)
	{}
	
	void init() {	
		timer_base->SINGLE.PER = 20000;	// 20 MHz clock counts to 20000 in 1ms
		timer_base->SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV1_gc;	// Prescaler of 1
		timer_base->SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm;		// Enable overflow interrupt
		timer_base->SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;		// Start timer
	}
	
	uint32_t millis() {
		return ticks;
	}

	uint32_t micros() {
		return (uint32_t)((ticks * 1000) + (timer_base->SINGLE.CNT * 0.05));
	}
	
	// Overflow interrupt service routine.
	void ovf_isr() {
		ticks++;
		timer_base->SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // Clear OVF interrupt
	}
	
};


#endif /* TIMER_H_ */