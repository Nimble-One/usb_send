#Simple makefile for libusb application

PROGNAME=usb_send

CC=gcc
INC=/usr/include/libusb-1.0/
OBJ=main.o

$(PROGNAME): $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o $(PROGNAME)

$(OBJ): main.c
	$(CC) -c main.c -I$(INC)

clean :
	-rm *.o $(PROGNAME)
