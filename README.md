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

  run without installing service

    mftd -v


  install service

    mftd -i


  uninstall service

    mftd -u


Configuration
-------------

Most configuration options are included in the default configuration file mftd.ini. For more dhcp options please read OpenDHCP's pdf.


Directory Layout
----------------

###bin/

  *mftd.exe* 

  main executable
  

###etc/

  *mftd.ini*

  mftd will check for its configuration file in the same directory it is run from and then in ../etc


  *mftd.state*

  dhcp lease information stored in same directory as binary or config


  *mftd.htm*

  http data for refreshing lease page stored in the same directory as binary or config


###log/ 

  logging is done automatically in a directory called log wherever the config file is found


###tmp/

  the make process stores built objects in tmp/build and the monitor module utilizes the tmp directory for storage 
