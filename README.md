# k64f-border-router
Border router application for the [FRDM K64F board] (https://developer.mbed.org/platforms/FRDM-K64F/)

#### Selecting the backhaul driver
Currently, the K64F border router application supports SLIP (IPv6 encapsulation over a serial line) and Ethernet backhaul connectivity. Which one is used is chosen through yotta configuration (field *backhaul-driver* in the *k64f-border-router* section). Value `SLIP` includes the SLIP driver, while the value `ETH` compiles the border router application with Ethernet backhaul support.

The MAC address on the backhaul interface can be manually defined through yotta config (field *backhaul-mac-src* value `CONFIG`). Alternatively, the MAC address provided by the board hardware can be used (field *backhaul-mac-src* value `BOARD`).

#### Configuring the border router application
The border router specific settings can be found in the *config.json* file under section *border-router*. The full list of all configuration options can be found in the file *config.json.example* in the root directory of the [Nanostack Border Router](https://github.com/ARMmbed/nanostack-border-router) project. The most common configuration options are explained in the table below.

| Field                               | Description                                                   |
|-------------------------------------|---------------------------------------------------------------|
| backhaul-bootstrap-mode             | Defines whether manually configured backhaul prefix is used, or a prefix learnt through IPv6 neighbor discovery should be used. Allowed values are: `NET_IPV_BOOTSTRAP_STATIC` and `NET_IPV_BOOTSTRAP_AUTONOMOUS`. The SLIP backhaul connectivity is NOT supported to work in the autonomous mode, and the serial connection between the border router application and an external router must be configured manually. |
| backhaul-prefix                     | The IPv6 prefix (of length 64) assigned to and advertised on the backhaul (Ethernet or serial line) interface. Example format: `fd00:1:2::` |
| backhaul-default-route              | The default route (prefix and prefix length) where packets should be forwarded on the backhaul device, default: `::/0`. Example format: `fd00:a1::/10` |
| backhaul-next-hop                   | The next-hop value for the backhaul default route; should be a link-local address of a neighboring router, default: empty (on-link prefix). Example format: `fe80::1` |
| rf-channel                          | The wireless (6LoWPAN mesh network) radio channel the border router application listens on. |
| security-mode                       | The 6LoWPAN mesh network traffic (link layer) can be protected with the Private Shared Key (PSK) security mode, allowed values: `NONE` and `PSK`. |
| psk-key                             | 16-byte long private shared key to be used when the PSK security mode is used. Example format (hexadecimal byte values separated by commas inside brackets): `{0x00, ..., 0x0f}` |
| multicast-addr                      | Multicast forwarding is supported by default. This field defines the multicast address the border router application forwards multicast packets for (on the bakchaul and RF interfaces). Example format: `ff05::5` |

Example of the yotta configuration file (config.json):

```json
{
  "k64f-border-router": {
    "backhaul-driver": "ETH",
    "backhaul-mac-src": "CONFIG",
    "backhaul-mac": "{0x02, 0x00, 0x00, 0x00, 0x00, 0x01}"
  },
  "border-router": {
	"security-mode": "PSK",
	"psk-key-id": 1,
	"psk-key": "{0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf}",
	"backhaul-bootstrap-mode": "NET_IPV6_BOOTSTRAP_STATIC",
	"backhaul-prefix": "fd00:abcd::",
	"backhaul-default-route": "::/0",
	"backhaul-next-hop": "fe80::1",
	"rf-channel": 11,
	"multicast-addr": "ff05::4",
	//add_more_parameters_here...
  },
  "hardware" : {
    "pins" : {
	  "SERIAL_TX": "PTC17",
	  "SERIAL_RX": "PTC16"
    }
  }
}
```

#### Configuring the SLIP serial connection
By default, the UART3 serial line of the K64F board is used with the SLIP driver. See the *pins* section in the project's yotta configuration. To use a different UART line, replace the *SERIAL_TX* and *SERIAL_RX* values with correct TX/RX pin names. For the pin names of your desired UART line, please refer to the [K64F documentation] (https://developer.mbed.org/platforms/FRDM-K64F/).
