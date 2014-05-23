#
# Compilation settings
#
# Change or override these on the command line if you want to alter the executable name/service description

NAME     = mftd
DESC     = Monitor DNS Tunnel and DHCP Service

SERVICE_NAME="$(NAME)"
SERVICE_DISPLAY_NAME="$(DESC)"

# Module settings

# Change to 0 to remove a module, 1 to include

MONITOR=1 # Monitors and reports adapter and device status
FDNS=1    # Fake DNS 
TUNNEL=1  # TCP tunnel on demand
DHCP=1	  # DHCP server

# Path settings

SRCDIR   = src
CFGDIR   = etc
BINDIR   = bin
TMPDIR   = tmp
BUILDDIR = $(TMPDIR)/build
VPATH    = $(SRCDIR):$(BUILDDIR)

# Build tool settings

# Note: if you want to use mingw instead of mingw-w64 I believe adding -D_WIN32_WINNT=0x0501 to CFLAGS makes it build

# 32-bit
CC = i686-w64-mingw32-g++
LD = i686-w64-mingw32-g++

# 64-bit
#CC = x86_64-w64-mingw32-g++
#LD = x86_64-w64-mingw32-g++

RM       = /bin/rm -f
MKDIR    = mkdir -p
RMDIR    = rmdir --ignore-fail-on-non-empty
CP       = cp
INSTALL  = install
STRIP    = strip

# Rules

OBJS    = net.o monitor.o ini.o fdns.o tunnel.o dhcp.o core.o

NAMES   = $(patsubst %, $(BUILDDIR)/%, $(NAME))
OBJSS   = $(patsubst %, $(BUILDDIR)/%, $(OBJS))

CFLAGS  = -I$(SRCDIR)/include -DNAME=\""$(NAME)"\" -DSERVICE_NAME=\"$(SERVICE_NAME)\" -DSERVICE_DISPLAY_NAME=\"$(SERVICE_DISPLAY_NAME)\" -DMONITOR=$(MONITOR) -DFDNS=$(FDNS) -DTUNNEL=$(TUNNEL) -DDHCP=$(DHCP) -DCFGDIR=\"$(CFGDIR)\" -DTMPDIR=$(TMPDIR)
LDFLAGS = -static -lwsock32 -liphlpapi -lws2_32 -lpthread -lshlwapi

all: prep compile install

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $(BUILDDIR)/$@ $<

prep:
	$(MKDIR) $(BUILDDIR)

compile: $(OBJS)
	@echo $(CC)
	$(LD) -o $(NAMES) $(OBJSS) $(LDFLAGS)

install: compile
	$(INSTALL) -d $(BINDIR)
	$(INSTALL) -m 0755 $(NAMES) $(BINDIR)
	$(STRIP) $(BINDIR)/$(NAME)

uninstall:
	$(RM) $(BINDIR)/$(NAME)

clean:
	$(RM) $(OBJSS) $(NAMES) *.url *.state *.htm *~

mrclean: clean uninstall
	$(RMDIR) $(BINDIR)
	$(RMDIR) $(BUILDDIR)
