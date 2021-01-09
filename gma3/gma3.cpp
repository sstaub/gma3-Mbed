#include "gma3.h"
#include "DigitalIn.h"
#include "PinNames.h"
#include "SocketAddress.h"
#include "UDPSocket.h"
#include "nsapi_types.h"
#include <cstdint>

EthernetInterface eth;
UDPSocket udp;
SocketAddress GMA3_UDP;

string prefixName = "";
string pageName = "Page"; // Page name
string faderName = "Fader"; // Fader name
string executorKnobName = "Encoder"; // ExecutorKnob name
string keyName = "Key"; // Key name

void interfaceETH(uint8_t localIP[], uint8_t subnet[]) {
	SocketAddress LOCAL_IP(localIP, NSAPI_IPv4);
	SocketAddress SUBNET(subnet, NSAPI_IPv4);
	uint8_t gateway[4] = {0, 0, 0, 0};
	SocketAddress GATEWAY(gateway, NSAPI_IPv4);
	eth.set_network(LOCAL_IP, SUBNET, GATEWAY);
	eth.connect();
}

void interfaceUDP(uint8_t gma3IP[], uint16_t gma3UdpPort) {
	GMA3_UDP.set_ip_bytes(gma3IP, NSAPI_IPv4);
	GMA3_UDP.set_port(gma3UdpPort);
	udp.open(&eth);
	}

void sendUDP(string& msg) {
	udp.sendto(GMA3_UDP, msg.data(), msg.length());
	}

void sendUDP(string& msg, SocketAddress address) {
	udp.sendto(address, msg.data(), msg.length());
	}

void setPrefix(string prefix) {
	prefixName = prefix;
	}

void prefix(string prefix) {
	prefixName = prefix;
	}

void page(string page) {
	pageName = page;
	}

void fader(string fader) {
	faderName = fader;
	}

void executorKnob(string executorKnob) {
	executorKnobName = executorKnob;
	}

void key(string key) {
	keyName = key;
	}

Key::Key(PinName pin, uint16_t page, uint16_t key) : mypin(pin, PullUp) {
	last = mypin;
	this->page = page;
	this->key = key;
	}

void Key::update() {
	if (mypin != last) {
		string oscPattern = "/";
		if (!prefixName.empty()) oscPattern += prefixName + "/";
		oscPattern += pageName + to_string(page) + "/" + keyName + to_string(key);
		if (last == false) {
			last = true;
			message(oscPattern, BUTTON_RELEASE);
			}
		else {
			last = false;
			message(oscPattern, BUTTON_PRESS);
			}
		sendUDP(oscPattern);
		}
	} 


// TODO force output at the begin
Fader::Fader(PinName pin, uint16_t page, uint16_t key) : mypin(pin) {
	this->page = page;
	this->key = key;
	updateTime = us_ticker_read();
	}

void Fader::update() {
	if (updateTime + (FADER_UPDATE_RATE_MS * 1000) < us_ticker_read()) {
		int16_t raw = mypin.read_u16() >> 6; // reduce to 10bit
		raw = limit(raw, 8, 1015); // limit to top / bottom 2*FADER_THRESHOLD
		if (raw < (analogLast - FADER_THRESHOLD) || raw > (analogLast + FADER_THRESHOLD)) { // ignore jitter
			analogLast = raw;
			int32_t value = analogLast * 100 / 1015; // map to 0...100
			if (valueLast != value) {
				valueLast = value;
				string oscPattern = "/";
				if (!prefixName.empty()) oscPattern += prefixName + "/";
				oscPattern += pageName + to_string(page) + "/" + faderName + to_string(key);
				message(oscPattern, value);
				sendUDP(oscPattern);
				}
			}
		updateTime = us_ticker_read();
		}
	}


ExecutorKnob::ExecutorKnob(PinName pinA, PinName pinB, uint16_t page, uint16_t executorKnob, uint8_t direction) : mypinA(pinA, PullUp), mypinB(pinB, PullUp) {
	pinALast = mypinA;
	this->page = page;
	this->executorKnob = executorKnob;
	this->direction = direction;
	}

void ExecutorKnob::update() {
	encoderMotion = 0;
	pinACurrent = mypinA;	
	if ((pinALast) && (!pinACurrent)) {
		if (mypinB) {
			encoderMotion = - 1;
			}
		else {
			encoderMotion = 1;
			}
		if (direction == REVERSE) encoderMotion = -encoderMotion;
		}
	pinALast = pinACurrent;
	if (encoderMotion != 0) {
		string oscPattern = "/";
		if (!prefixName.empty()) oscPattern += prefixName + "/";
		oscPattern += pageName + to_string(page) + "/" + executorKnobName + to_string(executorKnob);
		message(oscPattern, encoderMotion);
		sendUDP(oscPattern);
		}
	}


CmdButton::CmdButton(PinName pin, string command) : mypin(pin, PullUp) {
	last = mypin;
	this->command = command;
	}

void CmdButton::update() {
	if (mypin != last) {
		string oscPattern = "/";
		if (!prefixName.empty()) oscPattern += prefixName + "/";
		oscPattern += "cmd";
		if (last == false) {
			last = true;
			}
		else {
			last = false;
			message(oscPattern, command);
			sendUDP(oscPattern);
			}	
		} 
	}



OscButton::OscButton(PinName pin, string pattern, int32_t integer32, uint8_t ip[], uint16_t port) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	this->integer32 = integer32;
	type = INT32;
	}

OscButton::OscButton(PinName pin, string pattern, float float32, uint8_t ip[], uint16_t port) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	this->float32 = float32;
	type = FLOAT32;
	}

OscButton::OscButton(PinName pin, string pattern, string msg, uint8_t ip[], uint16_t port) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	this->msg = msg;
	type = STRING;
	}

OscButton::OscButton(PinName pin, string pattern, flag_t flag, uint8_t ip[], uint16_t port) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	this->flag = flag;
	type = FLAG;
	}

OscButton::OscButton(PinName pin, string pattern, uint8_t ip[], uint16_t port) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	type = NONE;
	}

void OscButton::update() {
	if (mypin != last) {
		string oscPattern = pattern;
		if (last == false) {
			last = true;
			if (type == INT32) {
				message(oscPattern, (int32_t)0);
				sendUDP(oscPattern, address);
				return;
				}
			if (type == FLOAT32) {
				message(oscPattern, 0.0f);
				sendUDP(oscPattern, address);
				return;
				}
			}
		else {
			last = false; // TODO case
			switch (type) {
				case INT32:
					message(oscPattern, integer32);
					break;
				case FLOAT32:
					message(oscPattern, float32);
					break;
				case STRING:
					message(oscPattern, msg);
					break;
				case FLAG:
					message(oscPattern, flag);
					break;
				case NONE:
					message(oscPattern);
					break;
				}
			sendUDP(oscPattern, address);
			}
		}
	} 


void message(string& osc, int32_t value) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add typee tag
	osc += ",i";
	osc += '\0';
	osc += '\0';
	// add value
	uint8_t *int32Array = (uint8_t *) &value; // itoa
	osc += int32Array[3];
	osc += int32Array[2];
	osc += int32Array[1];
	osc += int32Array[0];
	}

void message(string& osc, float value) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add typee tag
	osc += ",f";
	osc += '\0';
	osc += '\0';
	// add value
	uint8_t *floatArray = (uint8_t *) &value; // itoa
	osc += floatArray[3];
	osc += floatArray[2];
	osc += floatArray[1];
	osc += floatArray[0];
	}

void message(string& osc, string value) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add typee tag
	osc += ",s";
	osc += '\0';
	osc += '\0';
	fill = 4 - value.length() % 4;
	osc += value;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	}

void message(string& osc, flag_t flag) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add typee tag
	osc += ",";
	switch (flag) {
		case T:
			osc += 'T';
			break;
		case F:
			osc += 'F';
			break;
		case N:
			osc += 'N';
			break;
		case I:
			osc += 'I';
			break;
		}
	osc += '\0';
	osc += '\0';
	}

void message(string& osc) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add typee tag
	osc += ",";
	osc += '\0';
	osc += '\0';
	osc += '\0';
	}
