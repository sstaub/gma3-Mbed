#include "gma3.h"
#include "DigitalIn.h"
#include "PinNames.h"
#include "SocketAddress.h"
#include "TCPSocket.h"
#include "UDPSocket.h"

EthernetInterface eth;
UDPSocket udp;
TCPSocket tcp;
TCPSocket tcpExtern;
SocketAddress GMA3_UDP;
SocketAddress GMA3_TCP;

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

void interfaceTCP(uint8_t gma3IP[], uint16_t gma3TcpPort) {
	GMA3_TCP.set_ip_bytes(gma3IP, NSAPI_IPv4);
	GMA3_TCP.set_port(gma3TcpPort);
	tcp.set_blocking(false);
	tcp.set_timeout(50);
	}

void sendUDP(string& msg) {
	udp.sendto(GMA3_UDP, msg.data(), msg.length());
	}

void sendUDP(string& msg, SocketAddress address) {
	udp.sendto(address, msg.data(), msg.length());
	}

void sendTCP(string& msg) {
	if (!tcp.open(&eth)) {
		tcp.open(&eth);
		};
	if (!tcp.connect(GMA3_TCP)) {
		tcp.connect(GMA3_TCP);
		tcp.send(msg.data(), msg.length());
		tcp.close();
		}
	}

void sendTCP(string& msg, SocketAddress address) {
	tcpExtern.set_blocking(false);
	tcpExtern.set_timeout(50);
	if (!tcpExtern.open(&eth)) {
		tcpExtern.open(&eth);
		};
	if (!tcpExtern.connect(address)) {
		tcpExtern.connect(address);
		tcpExtern.send(msg.data(), msg.length());
		tcpExtern.close();
		}
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

Key::Key(PinName pin, uint16_t page, uint16_t key, protocol_t protocol) : mypin(pin, PullUp) {
	last = mypin;
	this->page = page;
	this->key = key;
	this->protocol = protocol;
	}

void Key::update() {
	if (mypin != last) {
		string oscPattern = "/";
		if (!prefixName.empty()) oscPattern += prefixName + "/";
		oscPattern += pageName + to_string(page) + "/" + keyName + to_string(key);
		if (last == false) {
			last = true;
			message(oscPattern, BUTTON_RELEASE, protocol);
			}
		else {
			last = false;
			message(oscPattern, BUTTON_PRESS, protocol);
			}
		if (protocol == UDP) {
			sendUDP(oscPattern);
			return;
			}
		if ((protocol == TCP11) || (protocol == TCP10) || (protocol == TCP)) {
			sendTCP(oscPattern);
			}
		} 
	}

// TODO force output at the begin
Fader::Fader(PinName pin, uint16_t page, uint16_t key, protocol_t protocol) : mypin(pin) {
	this->page = page;
	this->key = key;
	this->protocol = protocol;
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
				message(oscPattern, value, protocol);
				if (protocol == UDP) {
					sendUDP(oscPattern);
					return;
					}
				if ((protocol == TCP11) || (protocol == TCP10) || (protocol == TCP)) {
					sendTCP(oscPattern);
					}
				}
			}
		updateTime = us_ticker_read();
		}
	}


ExecutorKnob::ExecutorKnob(PinName pinA, PinName pinB, uint16_t page, uint16_t executorKnob, uint8_t direction, protocol_t protocol) : mypinA(pinA, PullUp), mypinB(pinB, PullUp) {
	pinALast = mypinA;
	this->page = page;
	this->executorKnob = executorKnob;
	this->direction = direction;
	this->protocol = protocol;
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
			message(oscPattern, encoderMotion, protocol);
		if (protocol == UDP) {
			sendUDP(oscPattern);
			return;
			}
		if ((protocol == TCP11) || (protocol == TCP10) || (protocol == TCP)) {
			sendTCP(oscPattern);
			}
		}
	}


CmdButton::CmdButton(PinName pin, string command, protocol_t protocol) : mypin(pin, PullUp) {
	last = mypin;
	this->command = command;
	this->protocol = protocol;
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
			message(oscPattern, command, protocol);
			if (protocol == UDP) {
				sendUDP(oscPattern);
				return;
				}
			if ((protocol == TCP11) || (protocol == TCP10) || (protocol == TCP)) {
				sendTCP(oscPattern);
				}
			}	
		} 
	}


OscButton::OscButton(PinName pin, string pattern, int32_t integer32, uint8_t ip[], uint16_t port, protocol_t protocol ) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	this->integer32 = integer32;
	type = INT32;
	this->protocol = protocol;
	}

OscButton::OscButton(PinName pin, string pattern, float float32, uint8_t ip[], uint16_t port, protocol_t protocol ) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	this->float32 = float32;
	type = FLOAT32;
	this->protocol = protocol;
	}

OscButton::OscButton(PinName pin, string pattern, string msg, uint8_t ip[], uint16_t port, protocol_t protocol ) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	this->msg = msg;
	type = STRING;
	this->protocol = protocol;
	}

OscButton::OscButton(PinName pin, string pattern, flag_t flag, uint8_t ip[], uint16_t port, protocol_t protocol ) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	this->flag = flag;
	type = FLAG;
	this->protocol = protocol;
	}

OscButton::OscButton(PinName pin, string pattern, uint8_t ip[], uint16_t port, protocol_t protocol ) : mypin(pin, PullUp) {
	last = mypin;
	address.set_ip_bytes(ip, NSAPI_IPv4);
	address.set_port(port);
	this->pattern = pattern;
	type = NONE;
	this->protocol = protocol;
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
					message(oscPattern, integer32, protocol);
					break;
				case FLOAT32:
					message(oscPattern, float32, protocol);
					break;
				case STRING:
					message(oscPattern, msg, protocol);
					break;
				case FLAG:
					message(oscPattern, flag, protocol);
					break;
				case NONE:
					message(oscPattern, protocol);
					break;
				}
			if (protocol == UDP) {
				sendUDP(oscPattern,address);
				return;
				}
			if ((protocol == TCP11) || (protocol == TCP10) || (protocol == TCP)) {
				sendTCP(oscPattern, address);
				}
			//sendUDP(oscPattern, address);
			}
		}
	} 


void message(string& osc, int32_t value, protocol_t protocol) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add type tag
	osc += ",i";
	osc += '\0';
	osc += '\0';
	// add value
	uint8_t *int32Array = (uint8_t *) &value; // itoa
	osc += int32Array[3];
	osc += int32Array[2];
	osc += int32Array[1];
	osc += int32Array[0];
	switch (protocol) {
		case UDP:
			break;
		case TCP:
			break;
		case TCP10:
			tcpEncode(osc);
			break;
		case TCP11:
			slipEncode(osc);
			break;
		}
	}

void message(string& osc, float value, protocol_t protocol) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add type tag
	osc += ",f";
	osc += '\0';
	osc += '\0';
	// add value
	uint8_t *int32Array = (uint8_t *) &value; // itoa
	osc += int32Array[3];
	osc += int32Array[2];
	osc += int32Array[1];
	osc += int32Array[0];
	switch (protocol) {
		case UDP:
			break;
		case TCP:
			break;
		case TCP10:
			tcpEncode(osc);
			break;
		case TCP11:
			slipEncode(osc);
			break;
		}
	}

void message(string& osc, string value, protocol_t protocol) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add type tag
	osc += ",s";
	osc += '\0';
	osc += '\0';
	fill = 4 - value.length() % 4;
	osc += value;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	switch (protocol) {
		case UDP:
			break;
		case TCP:
			break;
		case TCP10:
			tcpEncode(osc);
			break;
		case TCP11:
			slipEncode(osc);
			break;
		}
	}

void message(string& osc, flag_t flag, protocol_t protocol) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add type tag
	osc += ",";
	switch (flag) {
		case T:
			osc += 'T';
		case F:
			osc += 'F';
		case N:
			osc += 'N';
		case I:
			osc += 'I';
		}
	osc += '\0';
	osc += '\0';
	switch (protocol) {
		case UDP:
			break;
		case TCP:
			break;
		case TCP10:
			tcpEncode(osc);
			break;
		case TCP11:
			slipEncode(osc);
			break;
		}
	}

void message(string& osc, protocol_t protocol) {
	// fill pattern with zeros
	uint8_t fill = 4 - osc.length() % 4;
	for (uint8_t i = 0; i < fill; i++) {
		osc += '\0';
		}
	// add type tag
	osc += ",";
	osc += '\0';
	osc += '\0';
	osc += '\0';
	switch (protocol) {
		case UDP:
			break;
		case TCP:
			break;
		case TCP10:
			tcpEncode(osc);
			break;
		case TCP11:
			slipEncode(osc);
			break;
		}
	}

void slipEncode(string& msg) {
	int length = msg.length();
	for (int16_t i = length - 1; i >= 0; i--) {
		if (msg[i] == END) {
			msg[i] = ESC_END;
			msg.insert(i, 1, ESC);
		}
		else if (msg[i] == ESC) {
			msg[i] = ESC_ESC;
			msg.insert(i, 1, ESC);
		}
	}
	msg.insert(0, 1, END);
	msg += END;
}

void slipDecode(string& msg) {
	bool flagESC = false;
	bool flagEND = false;
	int length = msg.length();
	for (int16_t i = length - 1; i >= 0; i--) {
		if (msg[i] == END) {
			msg.erase(i, 1);
		}
		if (msg[i] == ESC_END) {
			flagEND = true;
		}
		if (msg[i] == ESC_ESC) {
			flagESC = true;
		}
		if (msg[i] == ESC) {
			if (flagEND) {
				msg.erase(i + 1, 1);
				msg[i] = END;
				flagEND = false;
			}
			if (flagESC) {
				msg.erase(i + 1, 1);
				msg[i] = ESC;
				flagESC = false;
			}
		}
	}
}

void tcpEncode(string& msg) {
	int32_t length = msg.length();
	uint8_t *int32Array = (uint8_t *) &length;
	msg.insert(0, 1, int32Array[0]);
	msg.insert(0, 1, int32Array[1]);
	msg.insert(0, 1, int32Array[2]);
	msg.insert(0, 1, int32Array[3]);
};

void tcpDecode(string& msg) {
	msg.erase(0, 4);
};
