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

#include <Client.h>
#if defined(ARDUINO)
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#endif
#include "MQTTArduino.h"

void TimerInit(Timer* timer)
{
	timer->interval_end_ms = 0;
}


char TimerIsExpired(Timer* timer)
{
	return (timer->interval_end_ms > 0L) && (millis() >= timer->interval_end_ms);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
	timer->interval_end_ms = millis() + timeout;
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
	TimerCountdownMS(timer, timeout * 1000L);
}


int TimerLeftMS(Timer* timer)
{
	return timer->interval_end_ms - millis();
}


int arduino_read(Network* network, unsigned char* buffer, int len, int timeout_ms)
{
	int interval = 10;  // all times are in milliseconds
	int total = 0, rc = -1;
	int bytesRead = 0;
	Client* client = static_cast<Client*>(network->client);

	if (timeout_ms < 30)
		interval = 2;
	while (client->available() < len && total < timeout_ms)
	{
		delay(interval);
		total += interval;
	}
	if (client->available() >= len)
		rc = client->readBytes((char*)buffer, len);
	else {
		while (client->available() && bytesRead < len)
		{
			rc = bytesRead += client->readBytes((char*)buffer + bytesRead, len - bytesRead);
		}
	}
	return rc;
}


int arduino_write(Network* network, unsigned char* buffer, int len, int timeout_ms)
{
	Client* client = static_cast<Client*>(network->client);
	client->setTimeout(timeout_ms);

	int index = 0;
	int chunk = len;
	while (index < len) {
		if (network->chunkSize) {
			chunk = (network->chunkSize < len - index) ? network->chunkSize : len - index;
		}
		int bytesWritten = client->write((uint8_t*)buffer + index, chunk);
		if (bytesWritten == 0) {
			return -1;
		}
		index += bytesWritten;
	}
	return index;
}


void NetworkInit(Network* network, void* client, int chunkSize)
{
	network->client = client;
	network->chunkSize = chunkSize;
	network->mqttread = arduino_read;
	network->mqttwrite = arduino_write;
}


int NetworkConnect(Network* network, const char* addr, int port)
{
	Client* client = static_cast<Client*>(network->client);
	return client->connect(addr, port);
}


void NetworkDisconnect(Network* network)
{
	Client* client = static_cast<Client*>(network->client);
	client->stop();
}


int NetworkConnected(Network* network)
{
	Client* client = static_cast<Client*>(network->client);
	return client->connected();
}