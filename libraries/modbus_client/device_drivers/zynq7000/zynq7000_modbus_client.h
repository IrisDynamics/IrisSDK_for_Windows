/**
 * @file zynq7000_modbus_client.h
 * @author Dan Beddoes <dbeddoes@irisdynamics.com>
 * 
 * @brief  Zynq7000 device driver for Modbus client serial communication
 * 
 * The object implements a buffer queue to sequentially transmit and receive MODBUS RTU formatted data.
 * 
 * @version 2.2.0
    
    @copyright Copyright 2022 Iris Dynamics Ltd 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    For questions or feedback on this file, please email <support@irisdynamics.com>. 
 */

#pragma once


#include <xtmrctr.h>
#include <xil_exception.h>
#include <xuartns550_l.h>
#include <xtime_l.h>

#include "modbus_client.h"

#include "../../../../interrupts.h" //todo: is this allowed??


/**
 * @class Zynq7000_ModbusClient
 * @brief Extension of the ModbusClient virtual class that implements functions for the Zynq7000's UARTs, timers, and interrupts
 *
 * Zynq7000 Documentation:
 * UART:  https://www.xilinx.com/support/documentation/ip_documentation/axi_uart16550/v2_0/pg143-axi-uart16550.pdf
 * Timer: https://www.xilinx.com/support/documentation/ip_documentation/axi_timer/v2_0/pg079-axi-timer.pdf
 *
*/


class Zynq7000_ModbusClient : public ModbusClient {

	u32 my_clock_freq;

	u32 uart_base_address;	// to be offset by config registers
	u32 uart_ier_address;	// interrupt enable register
	u32 uart_iir_address;	// interrupt identification register
	u32 uart_lsr_address;	// line status register
	u32 uart_thr_address;	// transmitter holding register
	u32 uart_rbr_address;	// receiver buffer register


public:

	friend class InterruptSystem2;
	u8 uart_intr_id;


	Zynq7000_ModbusClient(int channel, u32 cycles_per_second) : ModbusClient(channel, (COUNTS_PER_SECOND / 1000000))
	{
		switch(channel){
		case 0:
			my_clock_freq		=	XPAR_PL_UART0_CLOCK_FREQ_HZ;
			uart_base_address 	=	XPAR_PL_UART0_BASEADDR;
			uart_intr_id		=	XPAR_AXI_INTC_0_PL_UART0_IP2INTC_IRPT_INTR;
			break;
		case 1:
			my_clock_freq		=	XPAR_PL_UART1_CLOCK_FREQ_HZ;
			uart_base_address 	=	XPAR_PL_UART1_BASEADDR;
			uart_intr_id		=	XPAR_AXI_INTC_0_PL_UART1_IP2INTC_IRPT_INTR;
			break;
		case 2:
			my_clock_freq		=	XPAR_PL_UART2_CLOCK_FREQ_HZ;
			uart_base_address 	=	XPAR_PL_UART2_BASEADDR;
			uart_intr_id		=	XPAR_AXI_INTC_0_PL_UART2_IP2INTC_IRPT_INTR;
			break;
		case 3:
			my_clock_freq		=	XPAR_PL_UART3_CLOCK_FREQ_HZ;
			uart_base_address 	=	XPAR_PL_UART3_BASEADDR;
			uart_intr_id		=	XPAR_AXI_INTC_0_PL_UART3_IP2INTC_IRPT_INTR;
			break;
		case 4:
			my_clock_freq		=	XPAR_PL_UART4_CLOCK_FREQ_HZ;
			uart_base_address 	=	XPAR_PL_UART4_BASEADDR;
			uart_intr_id		=	XPAR_AXI_INTC_0_PL_UART4_IP2INTC_IRPT_INTR;
			break;
		case 5:
			my_clock_freq		=	XPAR_PL_UART5_CLOCK_FREQ_HZ;
			uart_base_address 	=	XPAR_PL_UART5_BASEADDR;
			uart_intr_id		=	XPAR_AXI_INTC_0_PL_UART5_IP2INTC_IRPT_INTR;
			break;
		case 6:
			my_clock_freq		=	XPAR_PL_UART6_CLOCK_FREQ_HZ;
			uart_base_address 	=	XPAR_PL_UART6_BASEADDR;
			uart_intr_id		=	XPAR_AXI_INTC_0_PL_UART6_IP2INTC_IRPT_INTR;
			break;
		case 7:
			my_clock_freq		=	XPAR_PL_UART7_CLOCK_FREQ_HZ;
			uart_base_address 	=	XPAR_PL_UART7_BASEADDR;
			uart_intr_id		=	XPAR_AXI_INTC_0_PL_UART7_IP2INTC_IRPT_INTR;
			break;
		default:
			break;
		}

		uart_ier_address		= 	uart_base_address  + XUN_IER_OFFSET;	// interrupt enable register
		uart_iir_address		= 	uart_base_address  + XUN_IIR_OFFSET;	// interrupt identification
		uart_lsr_address		= 	uart_base_address  + XUN_LSR_OFFSET;	// line status
		uart_thr_address		= 	uart_base_address  + XUN_THR_OFFSET;	// transmitter holding register
		uart_rbr_address		= 	uart_base_address  + XUN_RBR_OFFSET;	// receiver buffer register

	}

	int id(){
		return channel_number;
	}



    void init(int baud) override {

    	adjust_baud_rate(baud);
    	//////////////////////////////////////////////////////////////////////////////
    	// https://modbus.org/docs/Modbus_over_serial_line_V1_02.pdf  (section 2.5.1, pg 12)
    	// The format ( 11 bits ) for each byte in RTU mode is:
    	// Coding System:  	8–bit binary
    	// Bits per Byte: 	1 start bit
    	//					8 data bits, least significant bit sent first
    	//					1 bit for parity completion (even parity)
    	//					1 stop bit
    	/////////////////////////////////////////////////////// Set up UART Format
    	Xil_Out32 (uart_base_address + XUN_LCR_OFFSET, XUN_LCR_8_DATA_BITS| XUN_LCR_ENABLE_PARITY | XUN_LCR_EVEN_PARITY);
    	//////////////////////////////////////////////////////////////////////////////

    	/////////////////////////////////////////////////////// Set up UART interrupt
    	u32 current_ier = Xil_In32(uart_ier_address);
    	Xil_Out32(uart_ier_address, current_ier | XUN_IER_RX_DATA);
    	InterruptSystem2::enable( uart_intr_id  );

    	// Enable FIFO, reset FIFOs
    	Xil_Out32 (uart_base_address + XUN_FCR_OFFSET, XUN_FIFO_ENABLE);
    	// Set for FIFO Rx trigger level 8, FIFOs enabled, and reset


    	// fifo 14
    	// (creates communication troubles on some motors with scheduler at 8220 hz - possible to do with the mid april modbus rework on the orca firmware)
//    	Xil_Out32 (uart_base_address + XUN_FCR_OFFSET, XUN_FIFO_RX_TRIG_LSB | XUN_FIFO_RX_TRIG_MSB | XUN_FIFO_TX_RESET | XUN_FIFO_RX_RESET | XUN_FIFO_ENABLE);

    	// fifo 8
    	Xil_Out32 (uart_base_address + XUN_FCR_OFFSET, XUN_FIFO_RX_TRIG_MSB | XUN_FIFO_TX_RESET | XUN_FIFO_RX_RESET | XUN_FIFO_ENABLE);

    	// fifo 4
//    	Xil_Out32 (uart_base_address + XUN_FCR_OFFSET, XUN_FIFO_RX_TRIG_LSB | XUN_FIFO_TX_RESET | XUN_FIFO_RX_RESET | XUN_FIFO_ENABLE);

    	reset_state();
    }


    u32 get_system_cycles() override  {
    	u64 now;
    	XTime_GetTime(&now);
    	return now;
    }

protected:

/*
 * When the transmitter FIFO and THRE interrupt are enabled (FCR0 = 1, IER1 = 1), transmit interrupts occur as
follows:
1. The transmitter holding register empty interrupt [IIR (3 −0) = 2] occurs when the transmit FIFO is empty. It is
cleared [IIR (3 −0) = 1] when the THR is written to (1 to 16 characters may be written to the transmit FIFO
while servicing this interrupt) or the IIR is read.
2. The transmitter holding register empty interrupt is delayed one character time minus the last stop bit time
when there have not been at least two bytes in the transmitter FIFO at the same time since the last time that
the FIFO was empty. The first transmitter interrupt after changing FCR0 is immediate if it is enabled.
TL16C550C
SLLS177I – MARCH 1994 – REVISED MARCH 2021 www.ti.com
24 Submit Document Feedback Copyright © 2021 Texas Instruments Incorporated
Product Folder Links: TL16C550C
 */

    /**
     * @brief
     */
    void tx_enable() override {
    	u32 current_ier = Xil_In32(uart_ier_address);
    	Xil_Out32(uart_ier_address, current_ier | XUN_IER_TX_EMPTY);
    }

    void tx_disable() override {
    	u32 current_ier = Xil_In32(uart_ier_address);
		Xil_Out32(uart_ier_address, current_ier & ~XUN_IER_TX_EMPTY);
    }


    void send_byte(uint8_t data) override {
    	Xil_Out32(uart_thr_address, data);
    }

    uint8_t receive_byte() override {
    	return Xil_In32(uart_rbr_address);
    }

    bool byte_ready_to_receive() {
    	return Xil_In32(uart_lsr_address) & XUN_LSR_DATA_READY;
    }



    void adjust_baud_rate(uint32_t baud_rate_bps) override {
//    	XUartNs550_SetBaud(uart_base_address, my_clock_freq, baud_rate_bps); //todo: use this instead?
//////////////////////////////

       	u32 Divisor = my_clock_freq / (16 * baud_rate_bps);
		u32 BaudLSB = Divisor & XUN_DIVISOR_BYTE_MASK;
		u32 BaudMSB = (Divisor >> 8) & XUN_DIVISOR_BYTE_MASK;

//		u64 baud = BaudMSB << 16 | BaudLSB;


		// Set DLAB to allow divisor latch access
		u32 LcrRegister = Xil_In32(uart_base_address + XUN_LCR_OFFSET);
		Xil_Out32(uart_base_address + XUN_LCR_OFFSET, LcrRegister | XUN_LCR_DLAB );
		// Set the baud Divisors to set rate, the initial write of 0xFF is
		// to keep the divisor from being 0 which is not recommended as per
		// the NS16550D spec sheet
		Xil_Out32(uart_base_address + XUN_DRLS_OFFSET, 0xFF);
		Xil_Out32(uart_base_address + XUN_DRLM_OFFSET, BaudMSB);
		Xil_Out32(uart_base_address + XUN_DRLS_OFFSET, BaudLSB);
		// Restore line control register to allow normal register access
		Xil_Out32(uart_base_address + XUN_LCR_OFFSET, LcrRegister );

//        log_event(EventList::Event::baud_rate_changed, state);
    }

    /**
     * @brief handles transmission and reception interrupts
     *
     * Transmitting interrupt loads as many bytes as it can into the FIFO. When all bytes are loaded, the interrupt will be disabled.
     *
     * Receiving interrupts is called when enough data has arrived at the rx FIFO to trigger the interrupt. The FIFO data is pulled out of the FIFO with the run_in() function.
     *
     */
	void uart_isr() {

		volatile u32 isr_status = Xil_In32(uart_iir_address) & XUN_INT_ID_MASK;

		// https://www.xilinx.com/support/documentation/ip_documentation/axi_uart16550/v2_0/pg143-axi-uart16550.pdf
		// table 2-8
		#define MODEM_STATUS			0b0000	// reset by reading modem status register
		#define TX_HOLDING_REG_EMPTY	0b0010	// reset by reading IIR register (if source of interrupt), or writing inro transmitter holding register
		#define CHARACTER_TIMEOUT		0b1100	// reset by reading receiver
		#define RX_DATA_AVAILABLE		0b0100	// reset by reading data from FIFO
		#define RX_LINE_STATUS			0b0110	// reset by reading LSR

		switch ( isr_status ) {

			case CHARACTER_TIMEOUT:
			case RX_DATA_AVAILABLE:

				while( byte_ready_to_receive() ) {
					receive();
				}
				break;

			case TX_HOLDING_REG_EMPTY:

				for(int i=0; i < XUN_FIFO_SIZE; i++){
					if (my_state == emission) send();
					else { break; }
				}
				break;

			case RX_LINE_STATUS:
			case MODEM_STATUS:
			default:
				increment_diag_counter(unhandled_isr);
				break;
		}
	}


};

//extern Zynq7000_ModbusClient modbus_client[3];
//extern Zynq7000_ModbusClient modbus_client[6];

