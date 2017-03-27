CC = gcc
CFLAGS = -g -Wall -lpthread

DEPS = calc.h util.h params.h table.h
OBJS = calc.o util.o params.c table.c

opcit: $(OBJS) opcit.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f *.o opcit
