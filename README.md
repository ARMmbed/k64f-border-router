# FRDM-K64F border router
This document describes how to configure, compile, and run a FRDM-K64F 6LoWPAN border router application on a [FRDM-K64F development board](https://www.mbed.com/en/development/hardware/boards/nxp/frdm_k64f/). Instructions for debugging and testing are also provided.

Note: this Border Router does not support Thread.

## Introduction
Border router is a network gateway between a wireless 6LoWPAN mesh network and a backhaul network. It controls and relays traffic between the two networks. In a typical setup, a 6LoWPAN border router is connected to another router in the backhaul network (over Ethernet or a serial line) which in turn forwards traffic to/from the Internet or a private company LAN, for instance.

![](images/frdm_k64f_br_role.png)

## Software components
The FRDM-K64F border router application consists of 4 software components as shown in the image below:

![](images/frdm_k64f_br_components.png)

* [Nanostack Border Router](https://github.com/ARMmbed/nanostack-border-router) - is the core IPv6 gateway and provides the mesh network functionality.
* [Atmel RF driver](https://github.com/ARMmbed/atmel-rf-driver) - is the driver for the Atmel AT86RF2xxx wireless 6LoWPAN shields.
* [Ethernet driver](https://github.com/ARMmbed/sal-nanostack-driver-k64f-eth) - is the Ethernet driver for the FRDM-K64F development board.
* [SLIP driver](https://github.com/ARMmbed/sal-stack-nanostack-slip) - is a generic Serial Line over IPv6 (SLIPv6) driver for mbedOS boards.

## Required hardware
* An FRDM-K64F development board (6LoWPAN border router) - to run the border router application.
* An mbed 6LoWPAN shield (AT86RF212B/[AT86RF233](http://uk.rs-online.com/web/p/radio-frequency-development-kits/9054107/)) - for wireless 6LoWPAN mesh connectivity.
* A micro-USB cable - to connect the development board to a PC for debugging and power.
* An Ethernet cable - to connect the development board to a backhaul network.

![](images/frdm_k64f_board_plus_shield.png)

## Required software
* [Git](https://git-scm.com/) - for fetching the source files from GitHub.
* [yotta](http://docs.yottabuild.org/#installing) - for building the FRDM-K64F border router application.
* [ARM compiler toolchain](http://ds.arm.com/downloads/compilers/) - for building the FRDM-K64F border router application.
* [PuTTY](http://www.chiark.greenend.org.uk/~sgtatham/putty/) - for viewing the debug output.

## Configuration
To configure the FRDM-K64F border router you need to make changes in the yotta configuration file `config.json` in the root directory of the source tree. For the complete list of configuration options, please refer to the [Nanostack Border Router](https://github.com/ARMmbed/nanostack-border-router) documentation.

```json
  "k64f-border-router": {
    "backhaul-driver": "ETH",
    "backhaul-mac-src": "CONFIG",
    "backhaul-mac": "{0x02, 0x00, 0x00, 0x00, 0x00, 0x01}"
  },
```

##### Backhaul connectivity
The FRDM-K64F border router application can be connected to a backhaul network. This enables you to connect the devices in a 6LoWPAN mesh network to the Internet or a private LAN. Currently, the application supports SLIP (IPv6 encapsulation over a serial line) and Ethernet backhaul connectivity. You can select your preferred option through the yotta configuration (field *backhaul-driver* in the *k64f-border-router* section). Value `SLIP` includes the SLIP driver, while the value `ETH` compiles the FRDM-K64F border router application with Ethernet backhaul support. You can define the MAC address on the backhaul interface manually through the yotta config (field *backhaul-mac-src* value `CONFIG`). Alternatively, you can use the MAC address provided by the development board (field *backhaul-mac-src* value `BOARD`).

##### Prefix and default route
You need to configure a default route on the backhaul interface to properly forward packets between the backhaul and the 6LoWPAN mesh network. In addition to this, you need to set a backhaul prefix. You can set both either manually through the yotta configuration or automatically at runtime (IPv6 autoconfiguration). For details on how to set the backhaul prefix and default route, please refer to the [Nanostack Border Router](https://github.com/ARMmbed/nanostack-border-router) documentation.

##### Note on the SLIP backhaul driver
You need to use the UART1 serial line of the K64F board with the SLIP driver. See the *pins* section in the project's yotta configuration. To use a different UART line, replace the *SERIAL_TX* and *SERIAL_RX* values with correct TX/RX pin names. For the pin names of your desired UART line, please refer to the [FRDM-K64F documentation](https://developer.mbed.org/platforms/FRDM-K64F/).

Example yotta configuration for the SLIP driver:

```json
  "hardware" : {
    "pins" : {
	  "SERIAL_TX": "PTE0",
	  "SERIAL_RX": "PTE1"
    }
  },
```

## Build instructions
1. Install [yotta](http://docs.yottabuild.org/#installing).
2. Install the necessary compiler toolchains.
3. Download the source code of the FRDM-K64F border router by using Git.
4. Modify the yotta configuration file (`config.json`) to reflect your network setup.
5. In the root directory, select the target device: `yotta target frdm-k64f-armcc` or `yotta target frdm-k64f-gcc`
6. Build the binary: `yotta build`

The binary will be created in the `/build/frdm-k64f-armcc/source/` directory.

## Running the border router application
Locate the binary file `k64f-border-route.bin` in the directory `build/frdm-k64f-armcc/source/` and copy the file to the USB mass storage root of the FRDM-K64F development board. It will be automatically flashed to the MCU. After flashing, the board will restart itself. To finalise the flashing, press the reset button on the development board. A green light should start blinking to indicate successful startup.

## Debugging the border router application
To view the debug output of the border router application, you need to connect to the to the development board using a terminal emulation program, such as PuTTY.

In PuTTY:

1. Enter your identified serial port to **Serial line** (for example `/dev/ttyACM0` in Linux or `COM19` in Windows).
2. Set the **Speed** to 115200. 
3. Go to the **Serial** section under **Connection**. 
4. Set the serial line as `115200 8N1 XON/XOFF`.

**Linux tip:** To see which serial port your FRDM-K64F development board is connected to:
```
$ dmesg | grep -i ttyACM
```

In the PuTTY main screen, save the session and click **Open**. This opens a console window showing debug messages from the application. If the console screen is blank, you may need to press the reset button of the board to see the debug information. The debug prints in the console look something like this:

```
[DBG ][app ]: Using ETHERNET backhaul driver...
[DBG ][brro]: Backhaul driver ID: 0
[DBG ][brro]: RF device ID: 1
[DBG ][brro]: RF interface ID: 0
[DBG ][brro]: Backhaul interface ID: 1
[DBG ][brro]: Backhaul bootstrap started
[DBG ][brro]: Backhaul bootstrap ready, IPv6 = fd00:db8:ff1::ff:fe00:1
[DBG ][brro]: Backhaul default route:
[DBG ][brro]:    prefix:   ::/0
[DBG ][brro]:    next hop: fe80::1
[DBG ][brro]: Backhaul interface addresses:
[DBG ][brro]:    [0] fe80::ff:fe00:1
[DBG ][brro]:    [1] fd00:db8:ff1::ff:fe00:1
[DBG ][brro]: Using 24GHZ radio, type = 2, channel = 24
[DBG ][brro]: RF bootstrap ready, IPv6 = fd00:db8::ff:fe00:face
[DBG ][brro]: RF interface addresses:
[DBG ][brro]:    [0] fe80::ff:fe00:face
[DBG ][brro]:    [1] fe80::fec2:3d00:4:b8a8
[DBG ][brro]:    [2] fd00:db8::ff:fe00:face
```

## Testing the border router application using Linux
This section describes how to manually verify that the FRDM-K64F border router application is running correctly. For testing puposes, a working default configuration is already provided in the `config.json` file in the root directory and no yotta configuration changes are required.

1. Compile the border router application and flash it into the development board.
2. Connect the FRDM-K64F development board with an Ethernet cable to a router or switch.
3. Connect a PC running the Linux operating system to the same router/switch.
4. Connect the development board to the PC with a micro-USB cable. The board powers up and the USB drive gets mounted.
5. On the PC, use PuTTY to establish a serial connection to the board. 
6. After a successful connection, press the reset button on the board. Debug outputs should now be visible in the PuTTY console.
7. On the PC, issue the `ifconfig` command. You should find an IPv6 address with backhaul prefix `fd00:db8:ff1::/64` on your Ethernet interface.
8. Verify that the routes with the backhaul and RF interface prefixes have been added to your Linux routing table: `route -6`.
9. On the PC, ping the RF interface address of the border router: `ping6 -s 1456 fd00:db8::ff:fe00:face` and you should see pings going through.
10. You have verified that the FRDM-K64F border router application is up and running!

Example output of the `ifconfig` command on the PC:
```
eth0      Link encap:Ethernet  HWaddr 00:27:13:b4:25:46
          inet6 addr: fe80::227:13ff:feb4:2546/64 Scope:Link
          inet6 addr: fd00:db8:ff1:0:227:13ff:feb4:2546/64 Scope:Global
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          ...
```

Example output of the `route` command on the PC:
```
Kernel IPv6 routing table
Destination                    Next Hop                   Flag Met Ref Use If
fd00:db8::/64                  ::                         UAe  256 0     0 eth0
fd00:db8:ff1::/64              ::                         UAe  256 0     0 eth0
```
