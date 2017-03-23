CC = gcc
CFLAGS = -g -Wall

DEPS = calc.h util.h
OBJS = calc.o util.o

opcit: $(OBJS) opcit.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f *.o opcit
