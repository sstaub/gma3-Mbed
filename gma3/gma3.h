/*
gma3 OSC library for Mbed Ethernet UDP is placed under the MIT license
Copyright (c) 2020 Stefan Staub

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef GMA3_H
#define GMA3_H

#include "mbed.h"
#include <string>
#include "EthernetInterface.h"

using namespace std;

// helper functions
#define limit(x,low,high) ((x)<(low)?(low):((x)>(high)?(high):(x)))

// button values
#define BUTTON_PRESS    (int32_t)1
#define BUTTON_RELEASE  (int32_t)0

// encoder direction
#define FORWARD  0
#define REVERSE  1

// fader settings
#define FADER_UPDATE_RATE_MS  40 // update each 40ms
#define FADER_THRESHOLD       4 // Jitter threshold of the faders

// defines for SLIP
const char END = 0xC0; // indicates end of packet
const char ESC = 0xDB; // indicates byte stuffing
const char ESC_END = 0xDC; // ESC ESC_END means END data byte
const char ESC_ESC = 0xDD; // ESC ESC_ESC means ESC data byte

typedef enum protocolType {
	UDP,
	TCP10,
	TCP11,
	TCP
	} protocol_t;

typedef enum oscType {
	NONE,
	INT32,
	FLOAT32,
	STRING,
	FLAG,
	} osc_t;

typedef enum flagType {
	T,
	F,
	N,
	I
	} flag_t;

void interfaceETH(uint8_t localIP[], uint8_t subnet[]);

/**
 * @brief set UDP interface
 * 
 * @param gam3IP GrandMA3 console IP address
 * @param gma3UdpPort UDP interface
 */
void interfaceUDP(uint8_t gma3IP[], uint16_t gma3UdpPort = 8000);

/**
 * @brief set TCP interface
 * 
 * @param gam3IP GrandMA3 console IP address
 * @param gma3TcpPort OSC port off GrandMA3
 */
void interfaceTCP(uint8_t gma3IP[], uint16_t gma3TcpPort = 9000);

/**
 * @brief send an OSC message via UDP
 * 
 * @param msg OSC message
 * @param address SocketAddress for generic OSC buttons
 */
void sendUDP(string& msg);
void sendUDP(string& msg, SocketAddress address);

/**
 * @brief send an OSC message via TCP
 * 
 * @param msg OSC message
 * @param address SocketAddress for generic OSC buttons
 */
void sendTCP(string& msg);
void sendTCP(string& msg, SocketAddress address);

/**
 * @brief set the Prefix name
 * 
 * @param prefix 
 */
void prefix(string prefix);

/**
 * @brief set the Page name
 * 
 * @param page 
 */
void page(string page);

/**
 * @brief set the Fader name
 * 
 * @param fader 
 */
void fader(string fader);

/**
 * @brief set the ExecutorKnob name
 * 
 * @param executorKnob 
 */
void executorKnob(string executorKnob);

/**
 * @brief set the Key name
 * 
 * @param key 
 */
void key(string key);


/**
 * @brief Key object
 * 
 */
class Key {

	public:

		/**
		 * @brief Construct a new Key object
		 * 
		 * @param pin button pin
		 * @param page number of the page
		 * @param key number of the executor button
		 * @param protocol type of the used protocol, UDP or TCP
		 */
		Key(PinName pin, uint16_t page, uint16_t key, protocol_t protocol = UDP);

		/**
		 * @brief update the state of the Key button, must in loop()
		 * 
		 */
		void update();

	private:

		DigitalIn mypin;
		protocol_t protocol;
		uint16_t page;
		uint16_t key;
  	bool last;

	};


/**
 * @brief Fader object
 * 
 */
class Fader {

	public:

		/**
		 * @brief Construct a new Fader object
		 * 
		 * @param pin fader leveler pin
		 * @param page number of the page
		 * @param key number of the executor button
		 * @param protocol type of the used protocol, UDP or TCP
		 */
		Fader(PinName pin, uint16_t page, uint16_t key, protocol_t protocol = UDP);

		/**
		 * @brief update the state of the Key button, must in loop()
		 * 
		 */
		void update();

	private:

		AnalogIn mypin;
		protocol_t protocol;
		uint16_t page;
		uint16_t key;
		int16_t analogLast;
		uint32_t valueLast;
		uint32_t updateTime;

	};


/**
 * @brief ExecutorKnob object
 * 
 */
class ExecutorKnob {

	public:

		/**
		 * @brief Construct a new ExecutorKnob object
		 * 
		 * @param pinA pin A of the encoder
		 * @param pinB pin B of the encoder
		 * @param page number of the page
	 	 * @param executorKnob number of the executorKnob
		 * @param direction the direction for the encoder, can be FORWARD or REVERSE, depends on hardware alignment
		 * @param protocol type of the used protocol, UDP or TCP
		 */
		ExecutorKnob(PinName pinA, PinName pinB, uint16_t page, uint16_t executorKnob, uint8_t direction = FORWARD, protocol_t protocol = UDP);

		/**
		 * @brief update the output of the executorKnob, must be in loop()
		 * 
		 */
		void update();
	
	private:

		DigitalIn mypinA;
		DigitalIn mypinB;
		protocol_t protocol;
		uint16_t page;
		uint16_t executorKnob;
		bool pinALast;
		bool pinACurrent;
		uint8_t direction;
		int8_t encoderMotion;

	};


/**
 * @brief CndButton object
 * 
 */
class CmdButton {

	public:

		/**
		 * @brief Construct a new CmdButton object
		 * 
		 * @param pin button pin
		 * @param command string to command line
		 * @param protocol type of the used protocol, UDP or TCP
		 */
		CmdButton(PinName pin, string command, protocol_t protocol = UDP);

		/**
		 * @brief update the state of the Key button, must in loop()
		 * 
		 */
		void update();

	private:

		DigitalIn mypin;
		protocol_t protocol;
		string command;
  	bool last;

	};


/**
 * @brief osc button object
 * 
 */
class OscButton {

	public:

		/**
		 * @brief Construct a new Key object
		 * 
		 * @param pin button pin
		 * @param pattern OSC address
		 * @param integer32 optional integer value, must cast (int32_t) when using with a non matching size
		 * @param float32 optional float value
		 * @param msg optional string message
		 * @param flag optiona flag
		 * @param ip destination IP address
		 * @param externPort destination port
		 */
		OscButton(PinName pin, string pattern, int32_t integer32, uint8_t ip[], uint16_t port, protocol_t protocol = UDP);
		OscButton(PinName pin, string pattern, float float32, uint8_t ip[], uint16_t port, protocol_t protocol = UDP);
		OscButton(PinName pin, string pattern, string msg, uint8_t ip[], uint16_t port, protocol_t protocol = UDP);
		OscButton(PinName pin, string pattern, flag_t flag, uint8_t ip[], uint16_t port, protocol_t protocol = UDP);
		OscButton(PinName pin, string pattern, uint8_t ip[], uint16_t port, protocol_t protocol = UDP);

		/**
		 * @brief update the state of the Key button, must in loop()
		 * 
		 */
		void update();

	private:

		SocketAddress address;
		DigitalIn mypin;
		protocol_t protocol;
		string pattern;
		string msg;
		int32_t integer32;
		float float32;
		flag_t flag;
		osc_t type;
  	bool last;

	};

/**
 * @brief Creates osc messages with different data types
 * 
 * @param osc message
 * @param value integer32, float, string value
 * @param flag type value
 */
void message(string& osc, int32_t value, protocol_t protocol = UDP);
void message(string& osc, float value, protocol_t protocol = UDP);
void message(string& osc, string value, protocol_t protocol = UDP);
void message(string& osc, flag_t flag, protocol_t protocol = UDP);
void message(string& osc, protocol_t protocol = UDP);

/**
 * @brief Encode messages with SLIP
 * 
 * @param msg message
 */
void slipEncode(string& msg);

/**
 * @brief Decode SLIP encoded messages
 * 
 * @param msg message
 */
void slipDecode(string& msg);

/**
 * @brief Decode messages with Lengh identifier
 * 
 * @param msg message
 */
void tcpEncode(string& msg);

/**
 * @brief Encode messages with Lengh identifier
 * 
 * @param msg message
 */
void tcpDecode(string& msg);

#endif