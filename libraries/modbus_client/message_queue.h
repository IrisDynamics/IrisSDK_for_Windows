/**
 * @file message_queue.h
 * @author Kali Erickson <kerickson@irisdynamics.com>
 * 
 * @brief  Buffer queue for transmitting and receiving messages through the UART channels.
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


#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

#include "transaction.h"


/**
 * @class MessageQueue
 * @brief An array implemented queue of MODBUS RTU frame objects for sequential transmittion of commands to remote devices
*/
class MessageQueue {

    Transaction transaction_buffer[NUM_MESSAGES];  //!<Configure the max amount of messages in the buffer queue in the mb_config.h file
    int back_index = 0;       //!<index of next available empty spot
    int front_index = 0;      //!<index of item in front of queue
    int active_index = 0;

    Transaction error;   //!<error in the transaciton
    
public:
    /*@brief On construction the message queue and errors are reset.
    */
    MessageQueue(){
    	reset();
    }

    /**
     * @brief debugging information
     */
    void printme() {
#ifdef IRISCONTROLS

		PRINTDL("Queue Size: ", size());
		PRINTDL("front: ", front_index);
		PRINTDL("back: ", back_index);
		PRINTDL("active: ", active_index);
#endif 
    	for (int i = front_index; i != back_index; i++) {
    		i &= 0xF;
    		transaction_buffer[i].printme();
    	}
    	for (int i = 0; i < NUM_MESSAGES; i++) {
    		i &= 0xF;
    		transaction_buffer[i].printme();
    	}


    }

    /**
     * @brief reset all messages in the queue to be empty
     */
    void reset () {
        for(int i = 0; i < NUM_MESSAGES; i++) transaction_buffer[i].reset_transaction();
        error.reset_transaction();

        back_index = 0;
        front_index = 0;
        active_index = 0;
    }

    /**
     * @brief Adds the next message requested for transmission to the end of the queue if space is free
     * Returns false if the message was not added.
    */
    bool enqueue(Transaction message){
        if(!full()) {
        	transaction_buffer[back_index].reset_transaction();
        	transaction_buffer[back_index] = message;				// todo: get rid of this assignment...
        	transaction_buffer[back_index].mark_queued();
            back_index++;
            back_index &= (NUM_MESSAGES - 1); 
            return true;
        }
        return false;
    }

    /**
     * @brief used to check whether a message is ready to be dequeued
     */
    bool is_response_ready() {
    	if ( front_index != back_index && transaction_buffer[front_index].is_finished()) return true;
    	return false;
    }


    /**
     * @brief returns a pointer to the message now removed from the queue
     * 
    */
    Transaction * dequeue(){
    	Transaction * ret = 0;
    	if(size()) {
			ret = &transaction_buffer[front_index];
			front_index++;
			front_index &= (NUM_MESSAGES - 1);
			ret->mark_dequeued();
    	}
        return ret;
    }

    /**
     * @brief returns a pointer to the active transaction. No checking is done as to the state of the transaction
     */
    Transaction * get_active_transaction () {
    	return &transaction_buffer[active_index];
    }

    /**
     * @brief returns true when the transaction at the active_index is new and ready to start being sent
     * Advances the active_index when the current message is finished and marks the transaction as sent
     * ie this assumes the caller will transmit the message when this returns true
     * When this returns true, the transaction at active_index is ready to transmit, but hasn't been started yet
     */
    bool available_to_send() {

    	bool ret = false;
			
	if ( transaction_buffer[active_index].is_finished() || transaction_buffer[active_index].is_dequeued() ) {
		active_index++;
		active_index &= (NUM_MESSAGES-1); // mod
    	}
	if ((active_index == back_index) || transaction_buffer[active_index].is_active()) {
		// no new messages, or the current message is still active
		ret = false;
	}
	else if ( transaction_buffer[active_index].is_queued()) {
		// current message hasn't been sent yet, but was loaded and queued up
		ret = true;
		transaction_buffer[active_index].mark_sent();
	}

    	return ret;
    }

	/**
	 * @brief Determine the number of messages currently in the queue
	 * @return The number of messages in the queue
	*/
   int size(){
	   return (back_index - front_index) & (NUM_MESSAGES - 1);
   }

    /**
     * @brief Determine if the Message queue is full
     * @return True if the last enqueue filled the spot immediately 'before' the front of the queue, false otherwise.
    */
   bool full(){
       return size() >= (NUM_MESSAGES - 1);
   }



};

#endif

