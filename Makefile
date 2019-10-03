AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak


PROGS	= axis_server

CFLAGS+= -Wall -g -02
ifeq($(AXIS-BUILDTYPE),host)
LDFLAGS += -lcapturehost -ljpeg
else
LDFLAGS += -lcapture
endif

OBJS	= axis_server.o

all:	$(PROGS)


$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@
clean:
	rm -f $(PROGS) *.o core *.eap
	rm -f *tar
