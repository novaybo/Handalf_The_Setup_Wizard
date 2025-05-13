CC = gcc
CFLAGS = -LPDCurses -lpdcurses $(shell pkg-config --libs ncurses 2>/dev/null || echo "-lcurses")
TARGET = wizard

wizard: wizard.c
	$(CC) wizard.c -o wizard $(CFLAGS $(LDFLAGS))

run: wizard
	./wizard

clean:
	rm -f wizard
