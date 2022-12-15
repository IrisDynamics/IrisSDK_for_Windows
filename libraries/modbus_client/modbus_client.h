/**
 * @file modbus_client.h
 * @author Kali Erickson <kerickson@irisdynamics.com>
 * 
 * @brief Modbus client serial communication virtual class
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

#ifndef MODBUS_CLIENT_H_
#define MODBUS_CLIENT_H_

#include "message_queue.h"
#ifdef __MK20DX256__
#include <Arduino.h>
#endif


/**
 * @class ModbusClient
 * @brief Modbus client serial communication virtual class. To be implemented by device drivers.
 * 
 * This class uses a message buffer queue object, send/receive and timer dependent functions to be called by a derived class, and performs message validation.
 *
 * @section CLIENT_TIMER Client Timer
 *
 * The client uses a single timer to observe pauses in transmission, or determine when timeouts have occurred.
 * Only one delay timer / timeout timer can be active at once.
 * The timers present are:
 * # Response timeout - starts after all bytes of a message are sent to the transmitter, is cleared by receiving a byte, and expiry invalidates a message.
 * # Intercharacter timeout - starts after receiving a byte in the receiving state, is cleared when receiving the message's known payload, and expiry invalidates messages of known size, and triggers validation of unknown-size messages
 * # Interframe delay - starts following validation/invalidation of a message, is cleared only when it expires, and expiry returns the client to Idle
 * # Broadcast delay - starts following transmission of a broadcast message, is cleared when expiring and expiry enables the interframe delay
 *
 * */
class ModbusClient {

public:

    const int channel_number;               //!<the channels identifying number

    /**
     * @brief Description of the different diagnostic counters
    */
    typedef enum {

        message_sent_count                      = 5,
		return_bus_message_count 			    ,
        bytes_out_count			                ,
        bytes_in_count			                ,
		nothing_0								,
		return_server_exception_error_count	    = 10,
		return_server_NAK_count				    ,
		return_server_busy_count 			    ,
        unexpected_responder                    , //responder has wrong address or response to broadcast
		crc_error_count						    ,
		return_server_no_response_count		    = 15,
        unexpected_interchar                    ,
        ignoring_state_error                    , //entered ignoring state
		unhandled_isr							,

	} diag_counter_t;

    uint16_t diag_counters[20] = {0}; //!< 18 entry array of 16-bit serial line diagnostic counters


    /**
    * @brief construct ModbusClient object
    * @param _channel_number specify channel number, particularly relevant if there are multiple uart channels possible
    */                       
    ModbusClient(
        int _channel_number,
		uint32_t cycle_per_us
    ):
        channel_number(_channel_number),
    	my_cycle_per_us ( cycle_per_us ),
		repsonse_timeout_cycles  ( cycle_per_us * DEFAULT_RESPONSE_uS	),	// 100 milliseconds
		interchar_timeout_cycles ( cycle_per_us * DEFAULT_INTERCHAR_uS	),
		turnaround_delay_cycles  ( cycle_per_us * DEFAULT_TURNAROUND_uS	)
    {
    	adjust_interframe_delay_us  ( 			  DEFAULT_INTERFRAME_uS );
    }
    virtual ~ModbusClient(){}

    /**
     * @brief a debugging function which prints the state of the client
     */
    void printme() {
#ifdef IRISCONTROLS
	    PRINTDL( "timer enabled: " 	, my_enabled_timer);
        PRINTDL( "modbus state: "	, my_state );
		PRINTDL( "queue size: "		, get_queue_size());
    	messages.printme();
#endif
    }

    /**
      * @brief init tranceiver hardware
     */
     virtual void init(int baud) = 0;

////////////////////////////////////////////////////////
////////////////////////// State Machine Iteration ////
//////////////////////////////////////////////////////

    enum STATE_ID {
    	initial		= 20,
		idle,
		emission,
		reception,
		ignoring
    };

    volatile STATE_ID my_state = initial;


    /**
     * @brief brings the state machine back to an initial state
     */
    void reset_state () {
    	messages.reset();
    	my_state = initial;
    	enable_interframe_delay();
    }


    /**
     * @brief used to deactivate messages on the queue, poll the receiver hardware for buffered data, observe timeout timers
     * Should be polled at least as fast as messages are expected to arrive
     */
    void run_in() {

    	// Looking for message parsing? Check the application run_in() function (e.g. Actuator object).


    	// No timers are enabled
    	if ( my_enabled_timer == TIMER_ID::none ) {

    	}
    	// A timer is enabled
    	else {

    		Transaction * active_transaction = messages.get_active_transaction();


			TIMER_ID expired_timer = has_timer_expired();
			switch (expired_timer) {

			case TIMER_ID::repsonse_timeout:
				enable_interframe_delay();			// will allow run_out to send the next message once this expires (note this disables the current timer)
				increment_diag_counter(return_server_no_response_count);
				active_transaction->invalidate(Transaction::RESPONSE_TIMEOUT_ERROR);
				active_transaction->mark_finished();
				break;

			case TIMER_ID::interchar_timeout:

				enable_interframe_delay();			// will allow run_out to send the next message once this expires (note this disables the current timer)

				// If the length was unknown assume this was the expected termination of the response until it is validated
				if( !active_transaction->is_expected_length_known() ){
					validate_response(active_transaction);
				}
				// If the length was known and an interchar timeout occurred (ie the message got messed up)
				else {
					increment_diag_counter(unexpected_interchar);
					active_transaction->invalidate(Transaction::INTERCHAR_TIMEOUT_ERROR);
					increment_diag_counter(ignoring_state_error);
					my_state = ignoring;
				}

				active_transaction->mark_finished();

				break;


			case TIMER_ID::turnaround_delay:
				enable_interframe_delay();
				break;


			case TIMER_ID::interframe_delay:	// handled by run_out
			case TIMER_ID::none:				// No timers have expired, but one is enabled
			default:
				break;
			}

    	}

    }

    /**
     * @brief used to advance the message queue, start transmission of a new message, and observe interframe delay
     * Enables the response timer and starts the transmitter.
     * Should be polled at least as fast as messages are intended to be dispatched
     */
    void run_out () {
       
    	// If the interframe has expired, or there are no timers expired, check for a message to transmit
    	if ( my_enabled_timer == TIMER_ID::none	||  has_timer_expired() == TIMER_ID::interframe_delay) {
    		disable_timer();
    		if ( messages.available_to_send() ) {
                my_state = emission;
    			enable_response_timeout();
    			tx_enable();		// enabling the transmitter interrupts results in the send() function being called until the active message is fully sent to hardware
                increment_diag_counter(message_sent_count);    //temp? - for frequency benchmarking
    		}
    		else {
    			my_state = idle;
    		}
    	}
    }




////////////////////////////////////////////////////////////
///////////////////////// Queuing and Dequeuing Messages //
//////////////////////////////////////////////////////////

    /**
     * @brief enqueue a Transaction
     * @param message should be a populated Transaction object which will be copied into a Transaction in the message queue
     * @return 1 if succeeded in adding the message to the buffer, 0 if messages buffer was full
    */
    bool enqueue_transaction(Transaction message) {       
        return messages.enqueue(message);
    }

    /**
     * @brief Determine if the message at the front of the queue is ready to be claimed - ie has received a response or encountered an error
     * @return true if the message ready to be claimed
    */
    bool is_response_ready(){
        return messages.is_response_ready();
    }

    /**
     * @brief dequeue a transaction from the message queue
     * @return 0 when dequeue fails, or the address of the dequeued message otherwise
    */
    Transaction * dequeue_transaction(){
        return messages.dequeue();
    }

    /**
    * @brief get number of messages in the queue
    * @return True if the queue is empty (has no messages), False otherwise.
    */
    uint32_t get_queue_size(){
        return messages.size();
    }


/////////////////////////////////////////////////////////////
///////////////////////////////// Configuration Functions //
///////////////////////////////////////////////////////////

    /**
     * @brief Reconfigure the interframe delay in clock cycles
     * This is the minimum time that will be observed after receiving a message before a new message is transmitted
     * @param delay the minimum idle time between frames, in microseconds.
    */
    void adjust_interframe_delay_us(uint32_t delay) {
    	interframe_delay_cycles = my_cycle_per_us * delay;
    }

    /**
     * @brief return to default interframe delay
     */
    void adjust_interframe_delay_us() {
      	interframe_delay_cycles = my_cycle_per_us * DEFAULT_INTERFRAME_uS;
      }

    /**
     * @brief Adjust the baud rate
     * @param baud_rate the new baud rate in bps
    */
    virtual void adjust_baud_rate(uint32_t baud_rate) = 0;


    /// @brief Change the time required to elapse before a message is deemed failed. Used to reduce from the default after a handshake negotiates a higher baud
    void adjust_response_timeout    (u32 time_in_us) { 	repsonse_timeout_cycles = my_cycle_per_us * time_in_us; };
    /// @brief Change the time required to elapse between characters within a message before it is abandoned.
	void adjust_interchar_timeout   (u32 time_in_us) { interchar_timeout_cycles = my_cycle_per_us * time_in_us; };
	/// @brief Change the period of time observed between broadcast messages
	void adjust_turnaround_delay	(u32 time_in_us) { 	turnaround_delay_cycles = my_cycle_per_us * time_in_us; };

    /**
     * @brief Get the device's current system time in cycles
    */
    virtual uint32_t get_system_cycles() = 0;

    virtual void uart_isr() = 0;

    MessageQueue messages;            //!<a buffer for outgoing messages to facilitate timing and order of transmissions and responses

protected:


/////////////////////////////////////////////////////////////
///////////////////////////////// Hardware Implementations//
///////////////////////////////////////////////////////////


    /**
     * @brief Enable the transmitter in half-duplex systems (should disable the receiver)
     * This must enable the mechanism which calls the send() function when the transmitting hardware is capable of accepting data
     */
    virtual void tx_enable() = 0;

    /**
     * @brief Disable the transmitter in half-duplex systems (should enable the receiver)
     */
    virtual void tx_disable() = 0;

    /**
     * @brief Initiates transmission of a single byte.
	 * 		  Example: If using a UART, this function would write the passed byte to the UART TX FIFO
     */
    virtual void send_byte(uint8_t data) = 0;
    /**
     * @brief Returns the next byte to be received by the ModBus server.
	 * 		  Example: If using a UART, this function would read the UART FIFO upon receiving a byte, and return that byte.
     */
    virtual uint8_t receive_byte() = 0;
    /**
     * @brief should check receiver hardware to determine if at least one byte is ready to be read
     */
    virtual bool byte_ready_to_receive() = 0;

    public:
    /**
     * @brief Should be run when ready to send a new byte.
     *	Transitions to reception when done sending.
     */
    void send(){

		Transaction * active_transaction = messages.get_active_transaction();

		//send the current data byte
		uint8_t data = active_transaction->pop_tx_buffer();
		send_byte(data);
		increment_diag_counter(bytes_out_count);

		if ( active_transaction->is_fully_sent() ) {

			if ( active_transaction->is_broadcast_message() ){ //is it a broadcast message?
				enable_turnaround_delay();
			}else{
				enable_response_timeout();
			}
			tx_disable();   // enable receiver, disable further transmitter interrupts
			my_state = reception;
		}
    }


    /**
	 * @brief Should only be run when a new byte has been received.
	 * 		  Example: Call from UART byte received interrupt or when polling the hardware for data in the input fifo
	 */
	void receive() {

		Transaction * active_transaction = messages.get_active_transaction();


		uint8_t byte = receive_byte();
		active_transaction->load_reception(byte); //read the next byte from the receiver buffer. This clears the byte received interrupt    ??TODO: should we be loading here? it seems that in the overrun case we've already walked off the end of the array??
		increment_diag_counter(bytes_in_count);

		// If this was the last character for this message
		if (active_transaction->is_fully_received() )
		{
			enable_interframe_delay();// used to signal the earliest start time of the next message
			validate_response(active_transaction);// might transition to resting from connected
			active_transaction->mark_finished();
		}
		else {
			enable_interchar_timeout();
		}

    }

    /**
     * @brief Increment one of the serial line diagnostic counters.
     */
    void increment_diag_counter(diag_counter_t counter) {
    	diag_counters[counter]++;
    }



private:

	const u32 my_cycle_per_us;

	u32 repsonse_timeout_cycles ;
	u32 interchar_timeout_cycles;
	u32 turnaround_delay_cycles ;

	u32 interframe_delay_cycles  = 0;

	/// Time that the enabled timer was started


	volatile uint32_t timer_start_time;	// recorded in system cycles: must be checked as such


	/**
     * @brief Increment diagnostic counters and flag appropriate bits in the Transaction::reception_validity field based on the contents of the response
     * @param response a pointer to the transaction with the most recently received response in the MessageQueue
    */
    void validate_response(Transaction* response){

        // Check that destination and source addresses are the same
        if(response->get_tx_address() != response->get_rx_address()){   //if response to broadcast or incorrect responder
            increment_diag_counter(unexpected_responder);
            response->invalidate(Transaction::UNEXPECTED_RESPONDER);	// invalidates message
        }

        // Checking CRC
        if ( !response->check_rx_buffer_crc() ) {
            increment_diag_counter(crc_error_count);
            response->invalidate(Transaction::CRC_ERROR);				// invalidates message
        }

        // todo; other things that might invalidate a message


        // Increment counters for valid messages
		if ( response->is_reception_valid() ) {
			increment_diag_counter(return_bus_message_count);


			// Parse exception responses
			if (response->is_error_response()) {
				increment_diag_counter(return_server_exception_error_count);
				switch(response->get_rx_data()[0]){
					case 5: //exception code corresponding to NAK
						increment_diag_counter(return_server_NAK_count);
						break;
					case 6: //exception code corresponding to server busy error
						increment_diag_counter(return_server_busy_count);
				}
			}
		}
    }

/////////////////////////////////////////////////////////////
///////////////////////////////// The Timer ////////////////
///////////////////////////////////////////////////////////

	/**
	 * @brief represents which timer is active (ie only one timer is present at any given time)
	 */
	enum TIMER_ID {
		none,
		repsonse_timeout,
		interchar_timeout,
		turnaround_delay,
		interframe_delay

	};

	volatile TIMER_ID my_enabled_timer = none;

    /**
     * @brief Start/enable the interframe delay timer.
     *        The interframe delay is the minimum idle time between message frames
     *        To ensure accuracy, an interrupt based timer should be used if it is available
    */
    void enable_interframe_delay() {
    	timer_start_time = get_system_cycles();
    	my_enabled_timer = TIMER_ID::interframe_delay;
    }

    /**
     * @brief Start/enable the intercharacter timeout timer.
     *        The intercharacter timeout is the maximum time between characters in a message frame
    */
    void enable_interchar_timeout() {
    	timer_start_time = get_system_cycles();
    	my_enabled_timer = TIMER_ID::interchar_timeout;
    }
    /**
     * @brief Start/enable the response timeout timer.
     *        The response timeout is the maximum waiting time after transmitting a request before assuming the response will not be received
    */
    void enable_response_timeout() {
    	timer_start_time = get_system_cycles();
    	my_enabled_timer = TIMER_ID::repsonse_timeout;
    }
    /**
     * @brief Start/enable the turnaround delay timer.
     *        The turnarnound delay is the minimum time to wait after transmitting a broadcast message before moving to the next transmission
    */
    void enable_turnaround_delay() {
    	timer_start_time = get_system_cycles();
    	my_enabled_timer = TIMER_ID::turnaround_delay;
    }

    /**
     * @brief disables all timers
     */
    void disable_timer () { my_enabled_timer = TIMER_ID::none; }

    /**
     * @brief checks the time and returns the ID of any timer that has expired
     */
    TIMER_ID has_timer_expired () {

    	volatile uint32_t timer_start_time_l = timer_start_time;
    	volatile uint32_t tnow = get_system_cycles();

    	switch (my_enabled_timer) {
    	case TIMER_ID::repsonse_timeout : if ((uint32_t)(tnow - timer_start_time_l) > repsonse_timeout_cycles  ) return TIMER_ID::repsonse_timeout ; break;
    	case TIMER_ID::interchar_timeout: if ((uint32_t)(tnow - timer_start_time_l) > interchar_timeout_cycles ) return TIMER_ID::interchar_timeout; break;
    	case TIMER_ID::turnaround_delay : if ((uint32_t)(tnow - timer_start_time_l) > turnaround_delay_cycles  ) return TIMER_ID::turnaround_delay ; break;
    	case TIMER_ID::interframe_delay : if ((uint32_t)(tnow - timer_start_time_l) > interframe_delay_cycles  ) return TIMER_ID::interframe_delay ; break;
    	case TIMER_ID::none:
    	default:
    		break;
    	}

		return TIMER_ID::none;
    }

};


#endif

