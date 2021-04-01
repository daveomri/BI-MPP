CFLAFGS = -g -O0

all: main

main:   main.o usb_lib_usbip.o usbip_server.o
	gcc -o main main.o usb_lib_usbip.o usbip_server.o -l pthread

usb_lib_usbip.o: usb_lib_usbip.c usb_lib_usbip.h usbip_server.h
	gcc $(CFLAFGS) -c usb_lib_usbip.c

usbip_server.o: usbip_server.c usb_lib_usbip.h usbip_server.h
	gcc $(CFLAFGS) -c usbip_server.c

clean:
	rm -f *.o main

