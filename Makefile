CC ?= gcc

all: correlation$(SUFFIX)

correlation$(SUFFIX): main.o
	$(CC) -o $@ $<

.o.c:
	$(CC) -o $@ $<
