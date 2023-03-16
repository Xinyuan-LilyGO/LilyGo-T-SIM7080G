/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(__MQTT_ARDUINO_)
#define __MQTT_ARDUINO_


#if defined(__cplusplus)
extern "C" {
#endif

	/**
	* Countdown timer struct.
	*/
	typedef struct Timer
	{
		unsigned long interval_end_ms;
	} Timer;

	/**
	* Initialize countdown timer.
	* @param[in] timer Pointer to Timer struct
	*/
	void TimerInit(Timer* timer);

	/**
	* The countdown timer has expired.
	* @param[in] timer Pointer to Timer struct
	* @return 1 if countdown has expired, 0 otherwise.
	*/
	char TimerIsExpired(Timer* timer);

	/**
	* Start countdown in milliseconds.
	* @param[in] timer Pointer to Timer struct
	* @param[in] timeout Number of milliseconds to count down.
	*/
	void TimerCountdownMS(Timer* timer, unsigned int timeout);

	/**
	* Start countdown in seconds.
	* @param[in] timer Pointer to Timer struct
	* @param[in] timeout Number of seconds to count down.
	*/
	void TimerCountdown(Timer* timer, unsigned int timeout);

	/**
	* Get the number of milliseconds left in countdown.
	* @param[in] timer Pointer to Timer struct
	* @return Number of milliseconds left.
	*/
	int TimerLeftMS(Timer* timer);


	/**
	* Network struct for reading from and writing to a network connection.
	*/
	typedef struct Network
	{
		void* client; /**< The network client. */
		int chunkSize; /**< The chunk size to use when writing data, 0 for no limit. */

		/**
		* Read data from the network.
		* @param[in] network Pointer to the Network struct
		* @param[out] buffer Buffer that receives the data
		* @param[in] len Buffer length
		* @param[in] timeout_ms Timeout for the read operation, in milliseconds
		* @return Number of bytes read, or a negative value if there was an error
		*/
		int(*mqttread) (struct Network* network, unsigned char* buffer, int len, int timeout_ms);

		/**
		* Write data to the network.
		* @param[in] network Pointer to the Network struct
		* @param[in] buffer Buffer that contains data to write
		* @param[in] len Number of bytes to write
		* @param[in] timeout_ms Timeout for the write operation, in milliseconds
		* @return Number of bytes written, or a negative value if there was an error
		*/
		int(*mqttwrite) (struct Network* network, unsigned char* buffer, int len, int timeout_ms);
	} Network;

	/**
	* Read data from the network.
	* @param[in] network Pointer to the Network struct
	* @param[out] buffer Buffer that receives the data
	* @param[in] len Buffer length
	* @param[in] timeout_ms Timeout for the read operation, in milliseconds
	* @return Number of bytes read, or a negative value if there was an error
	*/
	int arduino_read(struct Network* network, unsigned char* buffer, int len, int timeout_ms);
	
	/**
	* Write data to the network.
	* @param[in] network Pointer to the Network struct
	* @param[in] buffer Buffer that contains data to write
	* @param[in] len Number of bytes to write
	* @param[in] timeout_ms Timeout for the write operation, in milliseconds
	* @return Number of bytes written, or a negative value if there was an error
	*/
	int arduino_write(struct Network* network, unsigned char* buffer, int len, int timeout_ms);

	/**
	* Initialize Network struct
	* @param[in] network Pointer to the Network struct
	* @param[in] client The network client
	* @param[in] network The chunk size to use when writing data, 0 for no limit
	*/
	void NetworkInit(Network* network, void* client, int chunkSize);

	/**
	* Connect to the specified address.
	* @param[in] network Pointer to the Network struct
	* @param[in] addr Destination address
	* @param[in] port Destination port
	* @return 0 if successfully connected, an error code otherwise
	*/
	int NetworkConnect(Network* network, const char* addr, int port);

	/**
	* Close the connection.
	* @param[in] network Pointer to the Network struct
	*/
	void NetworkDisconnect(Network* network);

	/**
	* Get the connection state.
	* @param[in] network Pointer to the Network struct
	* @return 1 if connected, 0 if not
	*/
	int NetworkConnected(Network* network);

#if defined(__cplusplus)
}
#endif

#endif
