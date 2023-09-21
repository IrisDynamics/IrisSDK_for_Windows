/**
 * @file iris_client_application.h
 * @author Kali Erickson <kerickson@irisdynamics.com>
 *
 * @brief  Establishes a Modbus connection status between a client and server device
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

#ifndef IRIS_CLIENT_APPLICATION_H_
#define IRIS_CLIENT_APPLICATION_H_

#include "modbus_client_application.h"
//#include "../../orca600_api/types.h"
#include "types.h"

 /**
  * @class IrisClientApplication
  * @brief Establishes a Modbus connection status between a client and server device.
  *
  *      The state machine will reach the connected state after completing the handshake sequence
  *           Communication Check:
  *               Succesful communication is established by receiving "req_num_discovery_pings" consecutive succesful responses to a query command.
  *			  Memory Map Synchronization:
  *			      Synchronize the local register content with the server register content by reading registers.
  *           Baud Rate and Messaging Delay Adjustment:
  *               Next, a command will be sent to adjust the value of the baud rate and messaging delay registers in the server device.
  *      If the handshake fails during any of these steps, a resting state will be entered before retrying from the first step.
  *
  *      The state machine will reset to the disconnected state if a number of consecutive failed messages are detected.
  *      The number of failed messages which constitutes a disconnection can be modified by adjusting the max_consec_failed_msgs variable from the ConnectionConfig struct before calling set_connection_config().
 */
class IrisClientApplication : public ModbusClientApplication {

	friend class ModBus_GUI;
	friend class Pneumatic_GUI;

public:

	IrisClientApplication(ModbusClient& _UART, const char* name, uint32_t _cycles_per_us) :

		ModbusClientApplication(_UART),
		UART(_UART),
		cycles_per_us(_cycles_per_us),
		my_name(name),
		pause_time_cycles(_cycles_per_us * DEFAULT_CONNECTION_PAUSE_uS)
    { }


    /**
     * @brief Configurable parameters for the handshake sequence and connection maintenance. Should be set when disabled
    */
    struct ConnectionConfig{
        uint8_t server_address        = 1;
        int req_num_discovery_pings   = 3; //3      //number of sucessful comms check messages required to move to next step in handshake sequence
        int max_consec_failed_msgs    = 10;      //number of failed/missed messages to trigger disconnect
        uint32_t target_baud_rate_bps = 625000;
        uint16_t target_delay_us      = 80;
        uint32_t response_timeout_us  = 8000;  /// this timeout will be used to override the default response timeout after a handshake succeeds and a new baud rate is negotiated.
    };

	ConnectionConfig connection_config;



    /**
     * @brief Error check and apply the handshake/connection configuration parameters passed in the ConnectionConfig struct
	 * 
	 * @param config ConnectionConfig object
     * @return 0 if one of the parameters was invalid and default values were used, 1 otherwise
    */
    int set_connection_config(IrisClientApplication::ConnectionConfig config){

        if(config.server_address < 1 || config.server_address > 247){
            return 0;   //todo : standardize error value
        }

        connection_config = config;

        return 1;   //todo : standardize error value
    }

    /**
     * @brief returns true when a new message was parsed or has failed since the last time this was called and returned true
     *
     * This function must return true when a new transaction has been claimed
     * since this function returned true last
     */
    virtual bool new_data() = 0;


    /**
     * @brief Determine whether a server has successfully connected with this client
     * @return true if the server is in the connected state, false otherwise
    */
    bool is_connected(){
        return connection_state == connected;
    }

    /**
     * @brief Determine if communication with a server is enabled or not
	 * 
	 * @return boolean - the enabled status (true if enabled, false otherwise)
    */
    bool is_enabled(){
        return enabled;
    }

    /**
     * @brief Enable communication with a server device. Allows handshake sequence to begin, enables transceiver hardware
    */
    void enable(){
        enabled = true;
    }

    /**
     * @brief Disable communication with a server device. Moves into disconnecting state where transceiver hardware will be disabled
    */
    void disable(){
        enabled = false;
        if(is_connected()){
            enqueue_change_connection_status_fn(connection_config.server_address, false, 0, 0);

        }
        disconnect();
    }

    /**
     * @brief Reset variables and move into the disconnected state
    */
    void disconnect(){
        //reset states
        connection_state 		= disconnected;
        cur_consec_failed_msgs 	= 0;
		UART.adjust_baud_rate(UART_BAUD_RATE);
		UART.adjust_interframe_delay_us();
		UART.adjust_response_timeout(DEFAULT_RESPONSE_uS);
		is_paused = true;// pause to allow server to reset to disconnected state

		start_pause_timer();
		desynchronize_memory_map();
	}
	typedef enum {
		disconnected = 50,	// reset state
		discovery = 51, 	// sending discovery pings, negotiating baud rate and delay
		synchronization = 52,
		negotiation = 53,
		connected = 54,	// streaming commands to the server
	} ConnectionStatus;

	volatile ConnectionStatus connection_state = disconnected;
protected:


	ModbusClient& UART;
	/**
	 * @brief Description of the possible connection states between the client and a server
	 *        Main state machine can be found in IrisClientApplication.
	 *        ModbusClient only responsible for moving to the 'disconnecting' state upon missed responses or errors
	*/
	const uint32_t cycles_per_us;
	const char* my_name;
	// Points to the last dequeued transaction from the modbus client
	Transaction* response;

	// This is used to determine when a connection has terminated and the ConnectionStatus should change to disconnecting
	int cur_consec_failed_msgs = 0;          //!< current number of consecutive failed messages

	enum connection_function_codes_e {
		change_connection_status = 65
	};

	/**
	 * @brief Determine the length of the request for an application specific function code
	 * 
	 * @param fn_code application specific function code
	 * @return int - length of request
	 */
	int get_app_reception_length(uint8_t fn_code) {
		switch (fn_code) {
		case IrisClientApplication::change_connection_status:
			return 12;
		default:
			return -1;
		}
	}
    
    /**
     * @brief Perform the next step in the handshake routine with a server device.
     * 
     * This function wants to progress from disconnected to connected through its various steps.
     * The state will remain in disconnected until the UARTs message queue is totally empty...ie all messages a received or timeout.
     * The state then becomes discovery where pings are sent until a number which is set in the config structure are successfully consecutively received
     * Following enough successful pings, we attempt to synchronize the server's memory map (if applicable) by queuing read register request(s).
     * If all requested read register messages are well received, a change connection status message is sent which requests the baud and interframe delay detailed by the config structure.
     * The state is now negotiation until the server responds. If the response is successful,
     *  the uart baud and interframe delays are adjusted based on what the server resports it realized
     *  and the state is now Connected.
     * If the negotiation fails, the state returns to discovery.
    */
	void modbus_handshake() {

		switch (connection_state) {
		case disconnected:
			if (UART.get_queue_size() == 0 && has_pause_timer_expired()) {
				is_paused = false;
				new_data(); // clear new data flag
				num_discovery_pings_received = 0;
				enqueue_ping_msg();

				connection_state = discovery;
			} else {
				// run_in() should be pulling any residual messages off the queue as they are received or timeout.
			}
			break;

		case discovery:

			// Note that the run_in() function should claim any responses that are ready,
			// set a new_data flag and save the transaction to the response member

			if (new_data()) {
				
				if (response->is_echo_response() && response->is_reception_valid()) {
					
					num_discovery_pings_received++;
					
					if (num_discovery_pings_received >= connection_config.req_num_discovery_pings) {
						synchronize_memory_map();			// loads many read messages onto the queue
						connection_state = synchronization;
						
					} else {
						enqueue_ping_msg();
					}
				}
				// new response was failed, or the wrong kind of response
				else {
					disconnect();
				}
			}
			break;


		case synchronization:

			if (new_data() && !response->is_reception_valid()) {
				// this allows queued data to timeout or be received before reattempting a connection
				disconnect();
			}
			else {
				
				if (UART.get_queue_size() == 0) {
					enqueue_change_connection_status_fn(
									connection_config.server_address,
									true,
									connection_config.target_baud_rate_bps,
									connection_config.target_delay_us);
					connection_state = negotiation;
				}
			}
			break;


		case negotiation:

			if (new_data()) {

				// Server responded to our change connection request with its realized baud and delay
				if (response->get_rx_function_code() == change_connection_status && response->is_reception_valid()) {
					uint8_t* rx_data = response->get_rx_data();
					UART.adjust_baud_rate(
											(uint32_t(rx_data[2]) << 24)
											| (uint32_t(rx_data[3]) << 16)
											| (uint32_t(rx_data[4]) << 8)
											| (uint32_t(rx_data[5]) << 0)); //set baud

					UART.adjust_interframe_delay_us(

							(uint16_t(rx_data[6]) << 8) | rx_data[7]); //set delay

					// Reduce timeouts
					UART.adjust_response_timeout(connection_config.response_timeout_us);

					connection_state = connected;

				}
				// Server failed to respond to our change connection request
				else {
					disconnect();
				}
			}
		case connected:
			// connection successful
			break;
		}
	}

	/**
	 * @brief can be overridden to request reading various holding registers which will be parsed and saved to the local version of the memory map
	 */
	virtual void synchronize_memory_map() {};

	/**
	 * @brief Can be overridden to reset the synchonised memory map
	 * Called when disconnect() is called
	 */
	virtual void desynchronize_memory_map() {};

	bool enabled = false;

private:

	int num_discovery_pings_received = 0;

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////  Pause Timer  ///
///////////////////////////////////////////////////////////////////////////

	bool is_paused = false;
	volatile uint32_t  pause_timer_start;
	const uint32_t pause_time_cycles;

	/**
	* @brief Start the pause timer. This can be done by saving the system time when the timer was started. Should not use interrupt timer
	*/
	virtual void start_pause_timer() {
		pause_timer_start = UART.get_system_cycles();
		is_paused = 1;
	}
	/**
	* @brief Check the progress of the 200 millisecond interval (pause timer)
	* @return The remaining time in the interval, 0 if the interval is not set or has finished
	*/
	virtual bool has_pause_timer_expired() {
		if (is_paused && (u32)(UART.get_system_cycles() - pause_timer_start) >= pause_time_cycles) {//(pause_time_us*CYCLES_PER_MICROSECOND)){
			return 1;
		}
		return 0;
	}


	/**
	* @brief Format a Transaction to check the communication with a certain server
	* @return 1 if the request was added to the queue, 0 if the queue was full
	*
	*/
	int enqueue_ping_msg() {
		uint8_t data[8] = { 1,2,4,8,16,32,64,128 };
		return return_query_data_fn(connection_config.server_address, data, 0); //pass in num_data as 0 so nothing from data array will be loaded into transmission
	}

	/**
	 * @brief
	 * @return 1 if the request was added to the queue, 0 if the queue was full
	*/
	int enqueue_adjust_baud_rate(uint32_t baud_rate_bps) {
		uint8_t data[4];
		data[0] = uint8_t(baud_rate_bps >> 24);
		data[1] = uint8_t(baud_rate_bps >> 16);
		data[2] = uint8_t(baud_rate_bps >> 8);
		data[3] = uint8_t(baud_rate_bps);

		return write_multiple_registers_fn(connection_config.server_address, uint16_t(MB_BAUD_HIGH), 2, data);
	}

	/**
	 * @brief
	 * @return 1 if the request was added to the queue, 0 if the queue was full
	*/
	int enqueue_adjust_interframe_delay(uint16_t delay_us) {
		return write_single_register_fn(connection_config.server_address, uint16_t(MB_DELAY), delay_us);
	}

	/**
	 * @brief Format a change_connection_status request, user-defined function code 65, and add the request to the buffer queue
	 * @param device_address
	 * @param connect true to connect the server, false to disconnect
	 * @param baud_rate_bps
	 * @param delay_us
	*/
	int enqueue_change_connection_status_fn(uint8_t device_address, bool connect, uint32_t baud_rate_bps, uint16_t delay_us) {
		Transaction transaction;
		uint16_t requested_state;
		connect ? requested_state = 0xFF00 : requested_state = 0;

		uint8_t data[8] = { uint8_t(requested_state >> 8),
							uint8_t(requested_state),
							uint8_t(baud_rate_bps >> 24),
							uint8_t(baud_rate_bps >> 16),
							uint8_t(baud_rate_bps >> 8),
							uint8_t(baud_rate_bps),
							uint8_t(delay_us >> 8),
							uint8_t(delay_us) };

		transaction.load_transmission_data(device_address, change_connection_status, data, 8, get_app_reception_length(change_connection_status));
		int check = UART.enqueue_transaction(transaction);
		return check;
	}

};
#endif
