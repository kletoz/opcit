CC = gcc
CFLAGS = -Wall

%.o: %.c libcalc.h
	$(CC) $(CFLAGS) -c -o $@ $<

calc: calc.o libcalc.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o calc
