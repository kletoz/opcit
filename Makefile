CC = gcc
CFLAGS = -Wall

%.o: %.c libcalc.h
	$(CC) $(CFLAGS) -c -o $@ $<

calc: calc.o libcalc.o
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f *.o calc
