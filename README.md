mftd
====

Monitor adapter, Fake DNS, Tunnel, and DHCP combined into one Windows Service

This program was written to provide a quick tunnel that sends all traffic to a certain port to another host. It includes a fake dns provider so that if hostnames are used then all requests to that hostname:port will be received by this program and tunneled properly. DHCP works if enabled in the source.

It is useful so you do not have to use Windows ICS and deal with its unreliable nature along side VPN traffic.


Build
-----

To build you will need GNU make, mingw-w64 (i686 or x64 for 32/64 bit) depending on what you comment/uncomment in the Makefile, and possibly cygwin for build tools. I believe it builds with regular mingw as well just follow the instructions in the Makefile.


Use
---

mftd -v : run without installing service

mftd -i : install service

mftd -u : uninstall service


Configuration
-------------

Most configuration options are included in the default configuration file mftd.ini. For more dhcp options please read OpenDHCP's pdf.
