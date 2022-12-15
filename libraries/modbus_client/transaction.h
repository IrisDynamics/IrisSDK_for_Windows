/**
 * @file Transaction.h
 * @author Kali Erickson <kerickson@irisdynamics.com>
 * 
 * @brief  MODBUS RTU frame object for outgoing and incoming requests.
 * 
 * @version 2.2.0
 * todo: describe how to wipe a Transaction object clean so it can be reused.
    
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
 *
 */

#ifndef TRANSACTION_H_
#define TRANSACTION_H_

#include <stdint.h>
#include "mb_crc.h"
#include "types.h"

/** For Debugging with Iriscontrols **/
//#define IRISCONTROLS
#if defined(IRISCONTROLS)
#include "../ic4_library/iriscontrols4.h"
#endif

/**
 * @class Transaction
 * @brief MODBUS RTU frame object for outgoing and respective incoming requests.
 * 
 * Formats output data and stores incoming responses in arrays of bytes according to MODBUS RTU specification.
*/
class Transaction {

    int tx_buffer_size = 0;               //The number of bytes stored in request
    uint8_t tx_buffer_index = 0;              //Index of the next byte from request to pop() and transmit
#ifdef __MK20DX256__
    uint8_t tx_buffer[64] = {0}; //The data to transmit
#elif defined(ATMEGA328)
    uint8_t tx_buffer[64] = { 0 }; //The data to transmit
#elif defined(ATTINY1617)
	uint8_t tx_buffer[64] = { 0 }; //The data to transmit
#else
	uint8_t tx_buffer[256] = { 0 }; //The data to transmit
#endif

    int rx_buffer_size = 0;               //The number of bytes stored in response
#if defined(ATMEGA328)
    uint8_t rx_buffer[64] = { 0 }; //The data to transmit
#elif defined(ATTINY1617)
	uint8_t rx_buffer[64] = { 0 }; //The data to transmit
#else
    uint8_t rx_buffer[256] = {0}; //The received response to the transmitted request
#endif 

    enum TRANSMIT_STATE {
    	unused	= 33,			// not a valid transaction to send
    	queued,					// has been loaded with data to send, but hasn't been marked as transmitted
		sent,					// has been transmitted (or is transmitting), but not marked as received or timed out
		finished,				// marked as done (either received or error encountered) as received or timed out
		dequeued,				// marked as having been removed from the queue (but not reset)
    };
    volatile TRANSMIT_STATE my_state = unused;

    volatile uint8_t reception_validity = 0b00000000;  //each bit of reception_validity indicates a different error in the response, bit = 0 means no error, bit = 1 means error detected
                                        //bit 0 - noise error
                                        //bit 1 - framing error
                                        //bit 2 - parity error
                                        //bit 3 - receiver overrun error
                                        //bit 4 - response timeout error
                                        //bit 5 - incorrect responder / response to broadcast
                                        //bit 6 - CRC error
                                        //bit 7 - invalid data <- need this??

    
    uint32_t ID = -1;
    static uint32_t id_assigner;

public:
	uint8_t rx_buffer_index = 0;              //Index of the next byte from response to pop() and examine/process

	volatile int reception_length; // expected length, in bytes, of the current request

    typedef enum  {

        R_OVERRUN_ERROR        	= 2,
        INTERCHAR_TIMEOUT_ERROR	= 3,
		RESPONSE_TIMEOUT_ERROR 	= 4,
		UNEXPECTED_RESPONDER   	= 5,
        CRC_ERROR              	= 6,
    } error_id;

    Transaction(){
    }

#ifdef IRISCONTROLS
    void printme() {
    	PRINTL ("===================================");
    	PRINTDL("ID             :"		, ID );
    	PRINTDL("tx buf size    :"		, tx_buffer_size );
    	PRINTDL("tx buf index   :"		, tx_buffer_index );
    	PRINTDL("tx fn code     :"		, tx_buffer[1]);
    	PRINTDL("rx buf size    :"		, rx_buffer_size );
    	PRINTDL("rx buf index   :"		, rx_buffer_index );
    	PRINTDL("state		    :"		, my_state );
    	PRINTDL("rx validity    :"		, reception_validity );
    	PRINTDL("rx_length      :"		, reception_length );
    }
#else
    void printme() {}
#endif



    /**
     * @brief Resets the buffer indexes and sizes to prepare for a new frame.
     */
    void reset_transaction() {
    	my_state = unused;
        reception_validity = 0;
        reception_length = 0;
        tx_buffer_index = 0;
        rx_buffer_index = 0;
        tx_buffer_size = 0;
        rx_buffer_size = 0;
    }

    /**
     * @brief Loads the passed data into a transmission
    */
    void load_transmission_data(uint8_t address, uint8_t function_code, uint8_t *data, int num_data, int num_expected_rx){
    	set_ID();
		tx_buffer_size = 4 + num_data;    //1 address byte + 1 function code byte + 2 CRC bytes = 4 bytes
		tx_buffer_index = 0;
		tx_buffer[0] = address;
		tx_buffer[1] = function_code;
		int i;
		for(i = 2; i < num_data + 2; i++) tx_buffer[i] = data[i - 2];
		uint16_t crc = ModbusCRC::generate(tx_buffer, tx_buffer_size - 2);
		tx_buffer[i++] = uint8_t(crc >> 8);
		tx_buffer[i] = uint8_t(crc);
		reception_length = num_expected_rx;
	}

    /**
     * @brief Loads the passed data into a transmission
     * Overloaded for variable length transmission
    */
    void load_transmission_data(uint8_t address, uint8_t function_code, uint8_t *framing_data, int num_framing_data, uint8_t *write_data, int num_write_data, int num_expected_rx){
    	set_ID();
        tx_buffer_size = 4 + num_framing_data + num_write_data;    //1 address byte + 1 function code byte + 2 CRC bytes = 4 bytes
        tx_buffer_index = 0;
        tx_buffer[0] = address;
        tx_buffer[1] = function_code;
        int i;
        for(i = 2; i < num_framing_data + 2; i++) tx_buffer[i] = framing_data[i - 2];
        for(i = num_framing_data + 2; i < num_write_data + num_framing_data + 2; i++) tx_buffer[i] = write_data[i - (num_framing_data + 2)];
        uint16_t crc = ModbusCRC::generate(tx_buffer, tx_buffer_size - 2);
        tx_buffer[i++] = uint8_t(crc >> 8);
        tx_buffer[i] = uint8_t(crc);
		reception_length = num_expected_rx;
    }


    /**
     * @brief should be called when this is placed in a queue
     */
    void mark_queued() {
    	my_state = queued;
    }
    /**
     * @brief should be called when transmission of this has started
     */
    void mark_sent() {
    	my_state = sent;
    }
    /**
     * @brief mark the message as having been finalized and ready for parsing (if valid)
    */
    void mark_finished(){
    	my_state = finished;
    }
    /**
	 * @brief mark the message as having been removed from the queue
	*/
	void mark_dequeued(){
		my_state = dequeued;
	}


    /**
     * @brief
     */
    bool is_queued() {
    	return my_state == queued;
    }
    /**
     * @brief returns true when the message has been sent but not finished
     */
    bool is_active() {
    	return my_state == sent;
    }
    /**
     * @brief returns true when the message has been finished sending
     */
    bool is_finished() {
    	return my_state == finished;
    }
    /**
     * @brief returns true when the message has been removed from the queue
     */
    bool is_dequeued() {
    	return my_state == dequeued;
    }



    /**
     * @brief returns true when the transaction has had all data popped off the output buffer
     */
    bool is_fully_sent() {
    	return	bytes_left_to_send() <= 0;
    }

    /**
     * @brief returns true when the transaction has received at least as many bytes as it expected
     * Does not indicate validity
     */
    bool is_fully_received() {
    	return
    		(get_rx_buffer_size() >= reception_length)
    		||
			(is_error_response() && get_rx_buffer_size() >= 5);
    }








    /**
     * @brief Determine the number of request bytes left to be transmitted
     * @return An integer - The number of request bytes left to transmit
    */
    int bytes_left_to_send(){
        return tx_buffer_size - tx_buffer_index;
    }

    /**
     * @brief Loads a single byte into into the response array
     * 
     * @param data	The data to be added to the response array
    */
    void load_reception(uint8_t data){
        rx_buffer[rx_buffer_size] = data;
        rx_buffer_size++;		// since this is u8 and rx_buffer is 256 large,  it will roll over without going out of bounds

    }



    /**
     * @brief Checks the received CRC value in the rx buffer against a newly generated CRC. Returns 1 if the values match.
     */
    int check_rx_buffer_crc() {

    	if(rx_buffer_size < 2) return 0; // prevent out of bounds array access

    	uint16_t crc_gen = ModbusCRC::generate(rx_buffer, rx_buffer_size - 2); // generate crc
    	uint16_t crc_rx = (rx_buffer[rx_buffer_size - 2] << 8) | rx_buffer[rx_buffer_size - 1]; // retrieve crc from response
    	if(crc_gen == crc_rx) return 1;
    	else return 0;
    }

    /**
     * @brief Access and remove data from the transmission
     * If the transmission array has data left, return the next piece of data then increment the transmission_index to the next byte.
     * @return The next data byte in the transmission
    */
    uint8_t pop_tx_buffer(){
    	return tx_buffer[tx_buffer_index++];
    }

    /**
     * @brief Access and remove data from the reception
     * If the reception array has data left, return the next piece of data then increment the reception_index to the next byte.
     * @return The next data byte in the reception
    */
    uint8_t pop_rx_buffer(){
    	return rx_buffer[rx_buffer_index++];
    }

    /**
     * @brief Determines the state of the request array
     * @return True if the index of the next data byte is equal to the total number of bytes because the array is 'empty'. False otherwise because there is still data left.
    */
    bool is_tx_buffer_empty(){
        return (tx_buffer_index >= tx_buffer_size) ? true : false;
    }

    /**
     * @brief Determines the state of the response array
     * @return True if the index of the next data byte is equal to the total number of bytes - the array is 'empty'. False otherwise - there is still data left.
    */
    bool is_rx_buffer_empty(){
        return (rx_buffer_index >= rx_buffer_size) ? true : false;
    }


    int is_expected_length_known () { return reception_length != -1; }


    /**
     * @brief Set the appropriate error bit in the reception_validity field to indicate an invalid response
     * @param error_id The bit number that represents the error in the reception_validity field
    */
    void invalidate(error_id error_id){
       reception_validity |= (1 << error_id);
    }

    /**
     * @brief is true when the message has not been invalidated
     */
    bool is_reception_valid(){
        return !reception_validity;
    }

    /**
     * @brief See if the request encountered errors during transmission or reception
     * Assumes the response has been fully received and processed. See the comment by the reception_validity field for value interpretation
     * @return The reception_validity field
    */
    int get_reception_validity(){
        return reception_validity;
    }




    bool is_error_response(){
        return (get_rx_function_code() & (1 << 7));
    }

    bool is_echo_response(){
        if(is_error_response()) return false;
        for(int i = 0; i < tx_buffer_size; i++){
            if(tx_buffer[i] != rx_buffer[i]) return false;
        }
        return true;
    }

    bool is_echo_response(int buffer_size){
        if(is_error_response()) return false;
        for(int i = 0; i < buffer_size; i++){
            if(tx_buffer[i] != rx_buffer[i]) return false;
        }
        return true;
    }

    /**
     * @brief true when is a broadcast (ie address is 0)
     */
    bool is_broadcast_message() {
    	return get_tx_address() == 0;
    }



    /**
     * @brief Set the id of the request
     * @param id the integer id of the request
    */
    void set_ID(){
        ID = id_assigner++;
    }

    /**
     * @brief Get the id of the request
     * @return an integer id that represents the request
    */
    int get_ID(){
        return ID;
    }

    /**
     * @brief Get the size of the request. If request has not been loaded, size will be 0
     * @return The number of bytes stored in the request array
     */
    int get_rx_buffer_size(){
        return rx_buffer_size;
    }

    /**
     * @brief Return the address of the server device that the request was adressed to. 
     * Assumes request has been loaded. If the request is a broadcast, the address will be 0.
     * @return The address of the server device that will receive the request.
    */
    uint8_t get_tx_address(){
        return tx_buffer[0];
    }

    /**
     * @brief Return the address of the server device that responded to the request. 
     * Assumes response address has been loaded.
     * @return The address of the server device that responded to the request.
    */
    uint8_t get_rx_address(){
        return rx_buffer[0];
    }

    uint8_t get_rx_function_code(){
        return rx_buffer[1];
    }

    uint8_t get_tx_function_code(){
        return tx_buffer[1];
    }

    uint8_t* get_rx_data(){
        return &rx_buffer[2];
    }

    uint8_t* get_tx_data(){
        return &tx_buffer[2];
    }

    int get_tx_buffer_size(){
        return tx_buffer_size;
    }


};

#endif

