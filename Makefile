AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak


#PROGS	= axis_server
PROGS	= axis_client
PKGS = glib-2.0 axevent
CFLAGS += -Wall -g -O2 -pthread -std=c99 -lm
ifeq ($(AXIS_BUILDTYPE),host)
LDFLAGS += -lcapturehost -ljpeg
else
LDFLAGS += -lcapture
endif
LDLIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_LIBDIR) pkg-config --libs $(PKGS))
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_LIBDIR) pkg-config --cflags $(PKGS))

#OBJS	= axis_server.o
OBJS	= axis_client.o
all: $(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(CFLAGS) $(LDLIBS) -o $@
	
clean: 
	rm -f $(PROGS) *.o core
	rm -f *.tar
