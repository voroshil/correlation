-include local.mk

CC ?= gcc


CFLAGS += -Wall -std=c99 -D_XOPEN_SOURCE=700
CFLAGS += $(INCLUDES)

LDFLAGS += -lncursesw -lm
LDFLAGS += $(LIBS)

TARGET := correlation$(SUFFIX)

all: $(TARGET)

$(TARGET): main.o
	$(CC) -static $< -o $@  $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $< 

clean:
	rm -f *.o *.d


.PHONY: all clean
