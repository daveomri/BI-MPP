obj-m := main.o
KDIR := /usr/src/linux-headers-5.4.0-54-generic
PWD := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules 
test-main: test-main.cpp
	g++ -Wall -pedantic test-main.cpp -o test-main