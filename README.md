# **Mbed OSC library for GrandMA3 consoles**
An object orientated library for Mbed [https://os.mbed.com](https://os.mbed.com) framework to control GrandMA3 consoles with OSC over Ethernet UDP and TCP. The goal of the library is to have a smart toolbox to create your own hardware which covers your needing by endless combinations of hardware elements.

## Mbed Studio IDE
The Mbed Studio IDE is recommanded for development. You can downlaod it from [https://os.mbed.com/studio/](https://os.mbed.com/studio/)

## Mbed boards
The developmnet was done with STM32 NUCLEO-F767ZI board which is very inexpensive ($ 24.- / € 23.-) but all other compatible boards should also work, have a look to [https://os.mbed.com/platforms/?q=&Communication=Ethernet](https://os.mbed.com/platforms/?q=&Communication=Ethernet)<br>
You can get the boards at Mouser in US [https://www2.mouser.com](https://www2.mouser.com) or Farnell in Europe [https://www.farnell.com](https://www.farnell.com) and other distributors.

## Mbed Version
The library is written for Mbed OS v6 and tested with the latest release v6.6

## Hardware
The library support hardware elements like encoders, faders, buttons with some helper functions. The library allows you to use hardware elements as an object and with the use of the helper functions, code becomes much easier to write and read and to understand.
- **Buttons**
	You can use every momentary push button on the market, e.g. MX Keys which are also used by MA Lighting, the keys are available with different push characters and have therefore different color markers. One pin must connect to a Digital Pin D*x* the other to ground.<br>
	**! A 100nF capitor is recommanded between the button pins !**<br>
- **Faders**
  Recommanded are linear faders with 10k Ohm from **Bourns** or **ALPS** which are available in different lengths and qualities.
	Beware that ARM boards like STM32-Nucleo use 3.3V, classic AVR boards like Arduino UNO use 5V. The leveler must connect to the Analog Pin A*x*. The other pin must connect to ground and 3.3/5 V depending on hardware alignment.<br>
	**! A 10nF capitor is recommanded between leveler and ground !**<br>

- **Rotary Encoders**
  You can use encoders from **ALPS** or equivalent.
	The middle pin of the encoders must connect to ground, the both other pins A/B must connect to Digital Pins D*x*.<br>
  **! Two 100nF capitors are recommanded between the button pin A/B and ground !**<br>

## Additional Advices for Analog Pins

- **STM32-Nucleo**
use IOREF Pin instead +3.3V to the top (single pin) of the fader (100%).
GND to the center button pin (2 pins, the outer pin is normaly for the leveler) of the fader (0%).

<img src="https://github.com/sstaub/gma3-Mbed/blob/master/images/gma3_nucleo.png?raw=true" width="85%"></img>

# Usage

## OSC settings
Please refer to the GrandMA3 manual for more information about using OSC on GrandMA3.
You can find general information about OSC on http://opensoundcontrol.org/

Here an example OSC setup
![GrandMA3 OSC Setup](https://github.com/sstaub/gma3-Mbed/blob/master/images/gma3_osc_setup.png?raw=true)<br>

## TCP support
This library also allows the use of TCP connections.

- This is a hard and dirty implementation because of missing stable TCP connections. If you e.g. press a button, a socket will open, send the message and then closes the socket. So you will see red lines in sysmon because of socket disconnections. I hope I will find  a better way later.
- You should use TCP when you want sure that the message is sended and received.
- TCP should not use for faders and encoders, because this cause a lot of traffic. You should use UDP instead.
- There is a bug in the TCP implementation of the GrandMA3 console. Normally you have two choices using for encoding/decoding TCP messages with OSC: SLIP (OSC spec 1.1) or Lenght (OSC spec 1.0) encoding, both doesn't work. Therefore you must use the ```TCP``` option (no encoding) instead of ```TCP10``` (OSC 1.0) and ```TCP11``` (OSC 1.1) in setup for the class members.

## Mbed Studio
The library is written and tested with the Mbed Studio IDE with Mbed OS v6.6
![Development on Mbed Studio](https://github.com/sstaub/gma3-Mbed/blob/master/images/gma3_development.png?raw=true)<br>
You have to start with an new program:<br>
<img src="https://github.com/sstaub/gma3-Mbed/blob/master/images/new_1.png?raw=true" width="30%"></img> <img src="https://github.com/sstaub/gma3-Mbed/blob/master/images/new_2.png?raw=true" width="30%"></img> 
<br>
Then put the gma3 library folder inside the project. You can copy the contents of the main.cpp example to prepared main.cpp<br>
The first build need a lot of time and processor power, be patient.

Refer also to Mbed Studio manual.

## Advice
If you have whishes for other functions or classes enter the discussion forum. If you find bugs make an issue, nobody is perfect.

## Ethernet configuration and initialization

1. You need to define IP addresses and ports 

- **localIP** - You need a static IP address for your Mbed board in the subnet range of network system
- **subnet** - A subnet range is necessary
- **gma3IP** - This is the GrandMA3 console IP address
- **gma3UdpPort** - This is the destination port of the GrandMA3 console


```cpp
// local network settings
uint8_t localIP[4] = {10, 101, 1, 201};
uint8_t subnet[4] = {255, 255, 0, 0};

// grandMA3 network settings
uint8_t gma3IP[4] = {10, 101, 1, 100};
uint16_t gma3UdpPort = 8000;
uint16_t gma3TcpPort = 9000;
```

In the beginning of main() you must start the network services.

```cpp
interfaceETH(localIP, subnet);
interfaceUDP(gma3IP, gma3UdpPort);
interfaceTCP(gma3IP, gma3TcpPort);
```

# Example
Here a small example (main.cpp in examples) with an additional button as an QLab GO button

```cpp
#include "mbed.h"
#include "gma3.h"

// local network settings
uint8_t localIP[4] = {10, 101, 1, 201};
uint8_t subnet[4] = {255, 255, 0, 0};

// grandMA3 network settings
uint8_t gma3IP[4] = {10, 101, 1, 100};
uint16_t gma3UdpPort = 8000;
uint16_t gma3TcpPort = 9000;

// QLab network settings
uint8_t qlabIP[4] = {10, 101, 1, 100};
uint16_t qlabUdpPort = 53000;

Key key201(D3, 1, 201);
Fader fader201(A0, 1, 201);
ExecutorKnob enc301(D0, D1, 1, 301);
CmdButton macro1(D2, "GO+ Macro 1", TCP); // TCP for GrandMA3
OscButton qlabGo(D4, "/go", qlabIP, qlabUdpPort, TCP11); // TCP 1.1 for QLab

int main() {
	prefix("gma3");
	interfaceETH(localIP, subnet);
 	interfaceUDP(gma3IP, gma3UdpPort);
	while (true) {
		key201.update();
		fader201.update();
		enc301.update();
		macro1.update();
		qlab.update();
		}
	}
```


# Helper functions

## **Interfaces**
### interfacETH()
```
void interfaceETH(uint8_t localIP[], uint8_t subnet[]);
```
This function is needed for initialize the ethernet connection to the GrandMA3 console.
Following settings must done first before ```ìnterfaceUDP()```:
- the local IP Address of the development board
- the subnet address

```cpp
interfaceETH(localIP, subnet);
```

### interfacUDP()
```
void interfaceUDP(uint8_t gma3IP[], uint16_t gma3UdpPort = 8000);
```
This function is needed for initialize the UDP socket for connection to the GrandMA3 console.
Following settings must done after ```interfaceETH()```:
- IP Address of the GrandMA3 console
- OSC UDP Port, set in the GrandMA3 console, standard is port 8000

```cpp
interfaceUDP(gma3IP, gma3UdpPort);
```

### interfacTCP()
```
void interfaceTCP(uint8_t gma3IP[], uint16_t gma3TcpPort = 9000);
```
This function is needed for initialize the UDP socket for connection to the GrandMA3 console.
Following settings must done:
- IP Address of the GrandMA3 console
- OSC TCP Port, set in the GrandMA3 console, standard is port 9000

```cpp
interfaceTCP(gma3IP, gma3TcpPort = 8010);
```

## Prefix name
```
void prefix(string prefix);
```
**prefix()** allows you to use a additional Prefix name, given in the GrandMA3 software.<br>
*! Must set inside ```main()```*<br>
- same Prefix String as configured in the console 

```cpp
prefix("gma3");
```

## Page name

```cpp
void page(string page);
```
**page()** allows you to use an alternative Page name, given in the GrandMA3 software.<br>
*! Must set inside ```main()```*<br>
- same Page String as configured in the console 

```cpp
page("Seite");
```

## Fader name

```cpp
void fader(string fader);
```
**fader()** allows you to use an alternative Fader name, given in the GrandMA3 software.<br>
*! Must set inside ```main()```*<br>
- same Fader String as configured in the console 

```cpp
fader("Regler");
```

## ExecutorKnob name

```cpp
void executorKnob(string executorKnob);
```
**executorKnob()** allows you to use an alternative ExecutorKnob name, given in the GrandMA3 software.<br>
*! Must set inside ```main()```*<br>
- same ExecutorKnob String as configured in the console 

```cpp
executorKnob("Drehregler");
```

## Key name

```cpp
void key(string key);
```
**key()** allows you to use an alternative Key name, given in the GrandMA3 software.<br>
*! Must set inside ```main()```*<br>
- same ExecutorKnob String as configured in the console 

```cpp
key("Taste");
```

# Classes
## Pin Names
Not all pin names follows the Arduino style (A0 - A5, D0 - D15), 
you must use for pin names of ST Zio connector the microcontroller pin names.<br>
For example instead of using ```D50``` as a pin name for the Nucleo-144 board you 
must use ```PG_3```, this is a small, reported bug.<br>
Have a look to the board documentation on the Mbed website.

## **Key**
With this class you can create Key objects which can be triggered with a button.

```cpp
Key(PinName pin, uint16_t page, uint16_t key, protocol_t protocol = UDP);
```
- **pin** are the connection Pin for the button hardware
- **page** is the page number of the executors, refer to the GrandMA3 manual
- **key** is the key number of the executors, refer to the GrandMA3 manual
- **protocol** is the protocol type you want to use, UDP, TCP, TCP10, TCP11, standard is UDP

Example, this should done before ```main()```

```cpp
Key key201(A1, 1, 201, TCP);
// executor button 201 on page 1 on pin A1 using TCP
```
To get the actual button state you must call inside the loop():

```cpp
void update();
```
Example, this must happen in the loop() 

```cpp
key201.update();
```

## **Fader**
This class allows you to control a fader containing  with a hardware (slide) potentiometer as an executor fader. 
```
Fader(PinName pin, uint16_t page, uint16_t key, protocol_t protocol = UDP);
```
- **pin** are the connection Analog Pin for the fader leveler
- **page** is the page number of the executors, refer to the GrandMA3 manual
- **fader** is the fader number of the executors, refer to the GrandMA3 manual
- **protocol** is the protocol type you want to use, UDP, TCP, TCP10, TCP11, standard is UDP

Example, this should done before ```main()```

```cpp
Fader fader201(A0, 1, 201);
// leveler is Analog Pin A0, page is 1 and executor number of the fader 201
```
To get the actual button state you must call inside the loop():

```cpp
void update();
```
Example, this must happen in the loop()

```cpp
fader201.update();
```

## **ExecutorKnob**
The ExecutorKnob class creates an encoder object which allows to control the executor knobs:

```cpp
ExecutorKnob(PinName pinA, PinName pinB, uint16_t page, uint16_t executorKnob, uint8_t direction = FORWARD, protocol_t protocol = UDP);
```
- **pinA** and **pinB** are the connection Pins for the encoder hardware
- **page** is the page number of the executors, refer to the GrandMA3 manual
- **executorKnob** is the number of the executor knob, refer to the GrandMA3 manual
- **direction** is used for changing the direction of the encoder to clockwise if pinA and pinB are swapped. The directions are FORWARD (standard) or REVERSE
- **protocol** is the protocol type you want to use, UDP, TCP, TCP10, TCP11, standard is UDP

Example, this should done before ```main()```

```cpp
ExecutorKnob enc301(A3, A4, 1, 301, REVERSE); 
// the encoder pins are A3/A4, page is 1, the number of the executorKnob is 301, encoder pins are swapped (REVERSE)
```

To get the actual encoder state you must call inside the loop():

```cpp
void update();
```
Example, this must happen in the loop() 

```cpp
enc301.update();
```

## **CmdButton**
With this class you can create Keya button which allows to send commands to the console.

```cpp
CmdButton(PinName pin, string command, protocol_t protocol = UDP);
```
- **pin** are the connection Pin for the button hardware
- **command** is a command string which should send to the console, refer also to the GrandMA3 manual
- **protocol** is the protocol type you want to use, UDP, TCP, TCP10, TCP11, standard is UDP

Example, this should done before ```main()```

```cpp
CmdButton macro1(A2, "GO+ Macro 1");
// button on pin A2, fires Macro 1
```
To get the actual button state you must call inside the loop():

```cpp
void update();
```
Example, this must happen in the loop()

```cpp
macro1.update();
```

## **OscButton**
With this class you can create generic buttons which allows you to control other OSC compatible software in the network like QLab. The class initializer is overloaded to allow sending different OSC data types: Interger 32 bit, Float, Strings or no data.
**! When using Interger or Float, 0 or 0.0 is send when releasing the button !**

```
OscButton(PinName pin, string pattern, int32_t integer32, IPAddress ip, uint16_t port, protocol_t protocol = UDP);
OscButton(PinName pin, string pattern, float float32, IPAddress ip, uint16_t port, protocol_t protocol = UDP);
OscButton(PinName pin, string pattern, string msg, IPAddress ip, uint16_t port, protocol_t protocol = UDP);
OscButton(PinName pin, string pattern, flag_t flag, uint8_t ip[], uint16_t port, protocol_t protocol = UDP);
OscButton(PinName pin, string pattern, IPAddress ip, uint16_t port, protocol_t protocol = UDP);
```
- **pin** the connection Pin for the button hardware
- **pattern** the OSC address pattern string
- **integer32** optional Integer data to send, you must cast this data e.g. ```(int32_t)1```
- **float32** optional Float data to send, you must cast this data e.g. ```1.0f```
- **msg** optional String to send
- **flag** optional OSC flags (T, F, N, I)
- **ip** optional destination IP address
- **port** optional destination port address
- **protocol** is the protocol type you want to use, UDP, TCP, TCP10, TCP11, standard is UDP

Example for Ethernet UDP using a button on Pin 0, this should done before the setup()

```cpp
uint8_t qlabIP[4] = {10, 101, 1, 100};
uint16_t qlabUDPPort = 53000; // QLab receive port
OscButton qlabGo(D4, "/go", qlabIP, qlabUdpPort, TCP11); // TCP 1.1
```
To get the actual button state you must call inside the loop():

```cpp
void update();
```
Example, this must happen in the loop()

```cpp
qlabGo.update();
```
