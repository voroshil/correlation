CC ?= gcc

CFLAGS += -Wall -std=c99 -I/usr/include/ncursesw -D_XOPEN_SOURCE=700
LDFLAGS += -lncursesw -lm

TARGET := correlation$(SUFFIX)

all: $(TARGET)

$(TARGET): main.o
	$(CC) -o $@ $<  $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $< 

clean:
	rm *.o *.d


.PHONY: all clean
