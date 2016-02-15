# k64f-border-router
Border router application for the FRDM K64F board

#### Selecting the backhaul driver
Currently, the K64F border router application supports SLIP (IP over serial line) and Ethernet backhaul connectivity. Which one is used is chosen through yotta configuration: field backhaul-driver in the 'k64f-border-router' section the config.json file. Value SLIP includes the SLIP driver, while value ETH compiles the border router application with Ethernet support.

The MAC address of the backhaul driver can be defined through yotta config ('backhaul-mac-src': 'CONFIG') or one provided by the board HW can be used ('backhaul-mac-src': 'BOARD'). 

#### Configuring the border router
The border router specific settings can be found in the *config.json* file under section *border-router*. The full list of all configuration options can be found in file *config.json.example* in the root directory of the *nanostack-border-router* project. The essential configuration options are explained in the table below.

| Field                               | Description                                                   |
|-------------------------------------|---------------------------------------------------------------|
| backhaul-bootstrap-mode             | Defines whether user configured backhaul prefix is used. Allowed values are:  NET_IPV_BOOTSTRAP_STATIC and NET_IPV_BOOTSTRAP_AUTONOMOUS. In the autonomous mode, an external router assigns a prefix for the application. |
| backhaul-prefix                     | The prefix assigned to and adversised on the backhaul interface. |
| backhaul-default-route              | The default route where packets should be forwarded on the backhaul device, default: ::/0. |
| backhaul-next-hop                   | The next-hop value for the backhaul default route; should be a link-local address of a neighboring router, default: empty (on-link prefix). |
| rf-channel                          | The radio channel the 6LoWPAN mesh network is formed. |
