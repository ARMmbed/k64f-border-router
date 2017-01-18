# FRDM-K64F border router

This document describes how to configure, compile, and run a FRDM-K64F 6LoWPAN border router application on a [FRDM-K64F development board](https://developer.mbed.org/platforms/FRDM-K64F/). 

<span class="notes">**Note:** This Border Router does not support Thread. For non-RTOS (yotta build), please follow onstructions in [Building with yotta](Building_with_yotta.md).</span>

## Introduction

Border router is a network gateway between a wireless 6LoWPAN mesh network and a backhaul network. It controls and relays traffic between the two networks. In a typical setup, a 6LoWPAN border router is connected to another router in the backhaul network (over Ethernet or a serial line) which in turn forwards traffic to/from the internet or a private company LAN, for instance.

![](images/frdm_k64f_br_role.png)

## Software components

The FRDM-K64F border router application consists of 4 software components as shown in the image below:

![](images/frdm_k64f_br_components.png)

* [Nanostack Border Router](https://github.com/ARMmbed/nanostack-border-router) is the core IPv6 gateway logic and provides the mesh network functionality.
* [Atmel RF driver](https://github.com/ARMmbed/atmel-rf-driver) is the driver for the Atmel AT86RF2xxx wireless 6LoWPAN shields.
* [Ethernet driver](https://github.com/ARMmbed/sal-nanostack-driver-k64f-eth) is the Ethernet driver for the FRDM-K64F development board.
* [SLIP driver](https://github.com/ARMmbed/sal-stack-nanostack-slip) is a generic Serial Line Internet Protocol version 6 (SLIPv6) driver for mbedOS boards.

## Required hardware

* Two FRDM-K64F development boards, one for the border router application and another one for [the 6LoWPAN mbed client application](https://github.com/ARMmbed/mbed-os-example-client).
* Two mbed 6LoWPAN shields (AT86RF212B/[AT86RF233](http://uk.rs-online.com/web/p/radio-frequency-development-kits/9054107/)) for wireless 6LoWPAN mesh connectivity.
 * Alternatively you can use [NXP MCR20A](http://www.nxp.com/products/software-and-tools/hardware-development-tools/freedom-development-boards/freedom-development-board-for-mcr20a-wireless-transceiver:FRDM-CR20A) shields.
 * See [Switching the RF shield](#switching-the-rf-shield)
* Two micro-USB cables to connect the development boards to a PC for debugging and power.
* An Ethernet cable to connect the development board to a backhaul network.

![](images/frdm_k64f_board_plus_shield.png)

## Required software

* [mbed-cli](https://github.com/ARMmbed/mbed-cli#installing-mbed-cli) command line interface.
* A compiler. Use one of the following:
    * [GCC ARM Embedded](https://launchpad.net/gcc-arm-embedded).
    * [ARM Compiler](https://developer.arm.com/products/software-development-tools/compilers/arm-compiler-5/downloads). (Requires license)
* [mbed account](https://www.mbed.com).
* A GitHub account.

## Optional software

* [PuTTY](http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html) for serial terminal emulation, see [serial connection settings](#serial-connection-settings).
* [Wireshark](https://www.wireshark.org/) for packet inspection and network debugging.
* [mbed Windows serial port driver](https://developer.mbed.org/handbook/Windows-serial-configuration), a serial driver for Windows to enable serial connections.


## Configuration

To configure the FRDM-K64F border router you need to make changes in the application configuration file `mbed_app.json` in the root directory of the source tree. For the complete list of configuration options, refer to the [Nanostack Border Router](https://github.com/ARMmbed/nanostack-border-router) documentation.

```json
{
    "config": {
        "heap-size": {
             "help": "The amount of static RAM to reserve for nsdynmemlib heap",
             "value": 50000
        },
        "debug-trace": "false",
        "trace-baud-rate": "115200",
        "backhaul-driver": "ETH",
        "backhaul-mac-src": "BOARD",
        "backhaul-mac": "{0x02, 0x00, 0x00, 0x00, 0x00, 0x01}"
        }
    }
}
```

#### Backhaul connectivity

The FRDM-K64F border router application can be connected to a backhaul network. This enables you to connect the devices in a 6LoWPAN mesh network to the internet or a private LAN. Currently, the application supports SLIP (IPv6 encapsulation over a serial line) and Ethernet backhaul connectivity. 

You can select your preferred option through the `mbed_app.json` file (field *backhaul-driver* in the *config* section). Value `SLIP` includes the SLIP driver, while the value `ETH` compiles the FRDM-K64F border router application with Ethernet backhaul support. You can define the MAC address on the backhaul interface manually (field *backhaul-mac-src* value `CONFIG`). Alternatively, you can use the MAC address provided by the development board (field *backhaul-mac-src* value `BOARD`). By default, the backhaul driver is set to be `ETH` and the MAC address source is `BOARD`. 

You can also set the bakchaul bootstrap mode (field *backhaul-bootstrap-mode*). By default, the bootstrap mode is set to be `NET_IPV6_BOOTSTRAP_AUTONOMOUS`. With autonomous mode, the border router learns the prefix information automatically from an IPv6 gateway in the ethernet/SLIP segment. Optionally, you can set the bootsrap mode to be `NET_IPV6_BOOTSTRAP_STATIC` which enables you to set up  a manual configuration of backhaul-prefix and default-route.

If you use static bootstrap mode, you need to configure a default route on the backhaul interface to properly forward packets between the backhaul and the 6LoWPAN mesh network. In addition to this, you need to set a backhaul prefix. Static mode creates a site-local IPv6 network from where packets cannot be routed outside.

 For more details on how to set the backhaul prefix and default route, refer to the [Nanostack Border Router](https://github.com/ARMmbed/nanostack-border-router) documentation.

When using the autonomous mode, you can set the `prefix-from-backhaul` option in the `mbed_app.json`file to `true` to use the same bakchaul prefix on the mesh network side as well. This allows for the mesh nodes to be directly connectable from the outside of the mesh network.

####Note on the SLIP backhaul driver

You need to use the UART1 serial line of the K64F board with the SLIP driver. See the *pins* section in the project's yotta configuration. To use a different UART line, replace the *SERIAL_TX* and *SERIAL_RX* values with correct TX/RX pin names. 
If you wish to use hardware flow control, set the configuration field `slip_hw_flow_control``to `true`. By default, it is set to `false`. Before using hardware flow control, make sure that the other end of your SLIP interface can handle flow control.

For the pin names of your desired UART line, refer to the [FRDM-K64F documentation](https://developer.mbed.org/platforms/FRDM-K64F/).

Example yotta configuration for the SLIP driver:

```json
  "config" : {
   	    "SERIAL_TX": "PTE0",
    	"SERIAL_RX": "PTE1",
    	"SERIAL_CTS": "PTE2",
    	"SERIAL_RTS": "PTE3"
  },
```

### Switching the RF shield

By default the application uses Atmel AT86RF233/212B RF driver. You can alternatively use FRDM-MCR20A shield also. Used RF driver is set in the `mbed_app.json` file.

To use the Atmel radio, use following:
```
        "radio-type":{
            "help": "options are ATMEL, MCR20",
            "value": "ATMEL"
        },
```

To use the NXP radio, use following:
```
        "radio-type":{
            "help": "options are ATMEL, MCR20",
            "value": "MCR20"
        },
```

After changing the radio, you need to recompile the application.

## Build instructions

1. Install [mbed-cli](https://github.com/ARMmbed/mbed-cli).
2. Clone the repository: `git clone git@github.com:ARMmbed/k64f-border-router.git`
3. Modify the `mbed_app.json` file to reflect to your network setup.
4. Deploy required libraries: `mbed deploy`
5. Generate mbed application root: `mbed new .`
6. Build: `mbed compile -m K64F -t GCC_ARM`

The binary will be generated into `.build/K64F/GCC_ARM/thread-testapp-private.bin`

## Running the border router application

1. Find the  binary file `k64f-border-router.bin` in the folder `.build/K64F/GCC_ARM/`.
2. Copy the binary to the USB mass storage root of the FRDM-K64F development board. It is automatically flashed to the target MCU. When the flashing is complete, the board restarts itself. Press the **Reset** button of the development board if it does not restart automatically.
3. The program begins execution.
4. Open the [serial connection](#serial-connection-settings), for example PuTTY.

## Serial connection settings

Serial connection settings for the Thread test application are as follows:

	* Baud-rate = 115200
	* Data bits = 8
	* Stop bits = 1
	* flow control = xon/xoff

If there is no input from the serial terminal, press the **Reset** button of the development board.

In the PuTTY main screen, save the session and click **Open**. This opens a console window showing debug messages from the application. If the console screen is blank, you may need to press the **Reset** button of the board to see the debug information. The prints for the border router look something like this in the console:

```
[INFO][app ]: Starting K64F border router...
[INFO][app ]: Using SLIP backhaul driver...
[INFO][app ]: Starting K64F border router...
[INFO][app ]: Using SLIP backhaul driver...
[INFO][addr]: Tentative Address added to IF 1: fe80::441:e2ff:fe12:faad
[INFO][addr]: DAD passed on IF 1: fe80::441:e2ff:fe12:faad
[INFO][addr]: Tentative Address added to IF 1: fd00:db8:ff1:0:441:e2ff:fe12:faad
[INFO][addr]: DAD passed on IF 1: fd00:db8:ff1:0:441:e2ff:fe12:faad
[INFO][brro]: Backhaul bootstrap ready, IPv6 = fd00:db8:ff1:0:441:e2ff:fe12:faad
[INFO][brro]: Backhaul interface addresses:
[INFO][brro]:    [0] fe80::441:e2ff:fe12:faad
[INFO][brro]:    [1] fd00:db8:ff1:0:441:e2ff:fe12:faad
[INFO][addr]: Address added to IF 0: fe80::ff:fe00:face
[INFO][br  ]: BR nwk base ready for start
[INFO][br  ]: Refresh xts
[INFO][br  ]: Refresh Prefixs
[INFO][addr]: Address added to IF 0: fd00:db8:ff1::ff:fe00:face
[INFO][addr]: Address added to IF 0: fe80::fec2:3d00:3:3503
[INFO][brro]: RF bootstrap ready, IPv6 = fd00:db8:ff1::ff:fe00:face
[INFO][brro]: RF interface addresses:
[INFO][brro]:    [0] fe80::ff:fe00:face
[INFO][brro]:    [1] fe80::fec2:3d00:3:3503
[INFO][brro]:    [2] fd00:db8:ff1::ff:fe00:face
[INFO][brro]: 6LoWPAN Border Router Bootstrap Complete.

```

