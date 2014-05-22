mftd
====

Monitor adapter, Fake DNS, Tunnel, and DHCP combined into one Windows Service


Build
-----

This only needs mingw-w64 (i686 or x64 for 32/64 bit) depending on what you comment/uncomment in the Makefile


Use
---

mftd -v : run without installing service

mftd -i : install service

mftd -u : uninstall service


Configuration
-------------

Most configuration options are included in the default configuration file mftd.ini. For more dhcp options please read OpenDHCP's pdf.
