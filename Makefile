#
# Compilation settings
#
# Change or override these on the command line if you want to alter the executable/service name/description

NAME     = mftd
DESC     = Monitor DNS Tunnel and DHCP Service

SERVICE_NAME="$(NAME)"
SERVICE_DISPLAY_NAME="$(DESC)"

#
# Module settings
#

# Change to 0 to remove a module, 1 to include

# Adapter/Communication analytics
MONITOR=1

# Fake DNS
FDNS=1

# TCP tunnel
TUNNEL=1

# DHCP server
DHCP=1

# HTTP server for local access to service status
HTTP=1

#
# Path settings
#

SRCDIR   = src
CFGDIR   = etc
BINDIR   = bin
TMPDIR   = tmp
LOGDIR   = log
BUILDDIR = $(TMPDIR)/build
VPATH    = $(SRCDIR):$(BUILDDIR)

#
# Build tool settings
#

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

#
# Build Rules
#

WFLAGS  = -Wno-write-strings
IFLAGS  = -I$(SRCDIR)/include
CFLAGS  = -DNAME=\""$(NAME)"\" -DSERVICE_NAME=\"$(SERVICE_NAME)\" -DSERVICE_DISPLAY_NAME=\"$(SERVICE_DISPLAY_NAME)\" -DMONITOR=$(MONITOR) -DFDNS=$(FDNS) -DTUNNEL=$(TUNNEL) -DDHCP=$(DHCP) -DHTTP=$(HTTP) -DCFGDIR=\"$(CFGDIR)\" -DLOGDIR=\"$(LOGDIR)\" -DTMPDIR=\"$(TMPDIR)\" $(WFLAGS) $(IFLAGS)
LDFLAGS = -static -lwsock32 -liphlpapi -lws2_32 -lpthread -lshlwapi
OBJS    = core.o util.o net.o ini.o

ifeq ($(MONITOR),1)
CFLAGS += -I$(SRCDIR)/include/metakit
MOBJS   = main.o net.o
MKOBJS  = column.o custom.o derived.o field.o fileio.o format.o handler.o persist.o remap.o std.o store.o string.o table.o univ.o view.o viewx.o
OBJS   += $(patsubst %, monitor/%, $(MOBJS))
OBJS   += $(patsubst %, monitor/metakit/%, $(MKOBJS))
endif
ifeq ($(FDNS),1)
OBJS   += fdns.o
endif
ifeq ($(TUNNEL),1)
OBJS   += tunnel.o
endif
ifeq ($(DHCP),1)
OBJS   += dhcp.o
endif 
ifeq ($(HTTP),1)
OBJS   += http.o
endif 

NAMES   = $(patsubst %, $(BUILDDIR)/%, $(NAME))
OBJSS   = $(patsubst %, $(BUILDDIR)/%, $(OBJS))

all: prep compile install

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $(BUILDDIR)/$@ $<

prep:
ifeq ($(MONITOR),1)
	$(MKDIR) $(BUILDDIR)/monitor/metakit
else
	$(MKDIR) $(BUILDDIR)
endif

compile: $(OBJS)
	$(LD) -o $(NAMES) $(OBJSS) $(LDFLAGS)

install: compile
	$(INSTALL) -d $(BINDIR)
	$(INSTALL) -d $(LOGDIR)
	$(INSTALL) -m 0755 $(NAMES) $(BINDIR)
	#$(STRIP) $(BINDIR)/$(NAME)

uninstall:
	-$(RM) $(BINDIR)/$(NAME)

clean:
	-$(RM) $(OBJSS) $(NAMES) $(TMPDIR)/*.htm $(TMPDIR)/*.state *~

mrclean: clean uninstall
ifeq ($(MONITOR),1)
	-$(RMDIR) $(BUILDDIR)/monitor/metakit
	-$(RMDIR) $(BUILDDIR)/monitor
endif
	-$(RMDIR) $(BUILDDIR)
