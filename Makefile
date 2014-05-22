
NAME = mftd
DESC = Monitor DNS Tunnel and DHCP Service

SERVICE_NAME=\"$(NAME)\"
SERVICE_DISPLAY_NAME=\""$(DESC)"\"

MONITOR=1
FDNS=1
TUNNEL=1
DHCP=0

CC = i686-w64-mingw32-g++
LD = i686-w64-mingw32-g++
#CC = x86_64-w64-mingw32-g++
#LD = x86_64-w64-mingw32-g++
#CFLAGS = -I./ -D_WIN32_WINNT=0x0501
CFLAGS = -I./ -DSERVICE_NAME=$(SERVICE_NAME) -DSERVICE_DISPLAY_NAME=$(SERVICE_DISPLAY_NAME) -DMONITOR=$(MONITOR) -DFDNS=$(FDNS) -DTUNNEL=$(TUNNEL) -DDHCP=$(DHCP)
LDFLAGS = -static -lwsock32 -liphlpapi -lws2_32 -lpthread -lshlwapi
OBJS = monitor.o ini.o fdns.o tunnel.o core.o

BINDIR = .
INSTALL = install
STRIP = strip
RM = /bin/rm -f
CP = cp

all: compile 

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< 

compile: $(OBJS)
	@echo $(CC)
	$(LD) -o $(NAME) $(OBJS) $(LDFLAGS)
#	$(CP) $(NAME) ..

install:
	$(INSTALL) -d $(BINDIR)
	$(INSTALL) -m 0755 $(NAME) $(BINDIR)
	$(STRIP) $(BINDIR)/$(NAME)

uninstall:
	$(RM) $(BINDIR)/$(NAME)

clean:
	$(RM) *.o *.url *.state *.htm *~ $(NAME)
