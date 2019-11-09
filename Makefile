AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak


PROGS	= axis_server
CFLAGS += -Wall -g -O2 -pthread -std=c99 -lm
ifeq ($(AXIS_BUILDTYPE),host)
LDFLAGS += -lcapturehost -ljpeg
else
LDFLAGS += -lcapture
endif

OBJS	= axis_server.o

all: $(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(CFLAGS) $(LDLIBS) -o $@
	
clean: 
	rm -f $(PROGS) *.o core
	rm -f *.tar
