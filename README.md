mftd
====

Monitor adapter, Fake DNS, Tunnel, and DHCP combined into one Windows Service


This program was written to provide a quick tunnel that sends all traffic to a certain port to another host. It includes a fake dns service so that if hostnames are used then all requests to that hostname will be received by this program and tunneled properly. Also included is a feature complete DHCP server forked from the OpenDHCP code base. Any service can be disabled or enabled in the configuration or even removed completely by editing the Makefile and rebuilding.

The entire program is less than 1 megabyte in size and includes built in service installation and unloading as well as a run directly mode (using -v). Logging is configurable and can be entirely shut off. Memory footprint is roughly 2 megabytes.


Release
-------

  *New*

   v0.3 release is now available. The binary is located at https://github.com/jwymanm/mftd/releases/download/v0.3/mftd.exe Put this in a folder called bin under the source tree and create tmp and log directories along side it. The ini should stay in etc/


Use
---

  run without installing service

    mftd -v


  install service

    mftd -i


  uninstall service

    mftd -u


Configuration
-------------

Many configuration options are included in the default configuration file *mftd.ini*. For more dhcp options please read OpenDHCP's pdf.


Directory Layout
----------------

###bin/

  *mftd.exe* 

  main executable
  

###etc/

  *mftd.ini*

  mftd will check for its configuration file in the same directory it is run from and then in ../etc


###log/ 

  logging is done in a directory called log either in ./ or ../log depending on if ini is in the same directory as the exe or not


###tmp/

  *mftd.state*

  dhcp lease state information (will be put in with bin + ini if all in same directory)

  *mftd.htm*

  http data for refreshing lease page (will be put in with bin + ini if all in same directory)

  *build/*

  the make process stores built object code in tmp/build


Build
-----

To build you will need GNU make, mingw-w64 (i686 or x64 for 32/64 bit) depending on what you comment/uncomment in the Makefile, and possibly cygwin for build tools. I believe it builds with regular mingw as well just follow the instructions in the Makefile. Once built it will work with or without cygwin.
