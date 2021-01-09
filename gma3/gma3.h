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

#include "SocketAddress.h"
#include "mbed.h"
#include <cstdint>
#include <string>
#include "EthernetInterface.h"

using namespace std;

// helpers
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

enum osc_t {
	NONE,
	INT32,
	FLOAT32,
	STRING,
	FLAG,
	};

enum flag_t {
	T,
	F,
	N,
	I
	};

void interfaceETH(uint8_t localIP[], uint8_t subnet[]);

/**
 * @brief set UDP interface
 * 
 * @param gam3IP GrandMA3 console IP address
 * @param gma3UdpPort UDP interface
 */
void interfaceUDP(uint8_t gma3IP[], uint16_t gma3UdpPort = 8000);

/**
 * @brief send an OSC message via UDP
 * 
 * @param msg OSC message
 * @param address SocketAddress for generic OSC buttons
 */
void sendUDP(string& msg);
void sendUDP(string& msg, SocketAddress address);

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
		 */
		Key(PinName pin, uint16_t page, uint16_t key);

		/**
		 * @brief update the state of the Key button, must in loop()
		 * 
		 */
		void update();

	private:

		DigitalIn mypin;
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
		 * @brief Construct a new Key object
		 * 
		 * @param pin fader leveler pin
		 * @param page number of the page
		 * @param key number of the executor button
		 */
		Fader(PinName pin, uint16_t page, uint16_t key);

		/**
		 * @brief update the state of the Key button, must in loop()
		 * 
		 */
		void update();

	private:

		AnalogIn mypin;
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
		 */
		ExecutorKnob(PinName pinA, PinName pinB, uint16_t page, uint16_t executorKnob, uint8_t direction = FORWARD);

		/**
		 * @brief update the output of the executorKnob, must be in loop()
		 * 
		 */
		void update();
	
	private:

		DigitalIn mypinA;
		DigitalIn mypinB;
		uint16_t page;
		uint16_t executorKnob;
		bool pinALast;
		bool pinACurrent;
		uint8_t direction;
		int8_t encoderMotion;

	};


/**
 * @brief Key object
 * 
 */
class CmdButton {

	public:

		/**
		 * @brief Construct a new Key object
		 * 
		 * @param pin button pin
		 * @param command string to command line
		 */
		CmdButton(PinName pin, string command);

		/**
		 * @brief update the state of the Key button, must in loop()
		 * 
		 */
		void update();

	private:

		DigitalIn mypin;
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
		OscButton(PinName pin, string pattern, int32_t integer32, uint8_t ip[], uint16_t port);
		OscButton(PinName pin, string pattern, float float32, uint8_t ip[], uint16_t port);
		OscButton(PinName pin, string pattern, string msg, uint8_t ip[], uint16_t port);
		OscButton(PinName pin, string pattern, flag_t flag, uint8_t ip[], uint16_t port);
		OscButton(PinName pin, string pattern, uint8_t ip[], uint16_t port);

		/**
		 * @brief update the state of the Key button, must in loop()
		 * 
		 */
		void update();

	private:

		SocketAddress address;
		DigitalIn mypin;
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
void message(string& osc, int32_t value);
void message(string& osc, float value);
void message(string& osc, string value);
void message(string& osc, flag_t flag);
void message(string& osc);

#endif