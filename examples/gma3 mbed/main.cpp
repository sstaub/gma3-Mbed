#include "mbed.h"
#include "gma3.h"

// local network settings
uint8_t localIP[4] = {10, 101, 1, 201};
uint8_t subnet[4] = {255, 255, 0, 0};

// grandMA3 network settings
uint8_t gma3IP[4] = {10, 101, 1, 100};
uint16_t gma3UdpPort = 8000;

// QLab network settings
uint8_t qlabIP[4] = {10, 101, 1, 100};
uint16_t qlabUdpPort = 53000;

Fader fader201(A0, 1, 201);
ExecutorKnob enc301(D0, D1, 1, 301);
CmdButton macro1(D2, "GO+ Macro 1");
Key key201(D3, 1, 201);
OscButton qlabGo(D4, "/go", qlabIP, qlabUdpPort);

int main() {
	prefix("gma3");
	interfaceETH(localIP, subnet);
	interfaceUDP(gma3IP, gma3UdpPort);
	while (true) {
		key201.update();
		fader201.update();
		enc301.update();
		macro1.update();
		qlabGo.update();
    }
	}

