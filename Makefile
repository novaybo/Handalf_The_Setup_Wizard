CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -LPDCurses $(shell pkg-config --libs ncurses 2>/dev/null || echo "-lcurses") -lcjson
TARGET = wizard

wizard: wizard.c
	$(CC) $(CFLAGS) wizard.c -o $(TARGET) $(LDFLAGS)

run: wizard
	sudo ./$(TARGET)

clean:
	rm -f $(TARGET)