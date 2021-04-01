/*************************************************************
 * COURSE WARE ver. 3.0
 * 
 * Permitted to use for educational and research purposes only.
 * NO WARRANTY OF ANY KIND.
 *
 * Faculty of Information Technology
 * Czech Technical University in Prague
 * Author: Miroslav Skrbek (C)2010,2011,2012,2020
 *         skrbek@fit.cvut.cz
 * 
 **************************************************************
 */

#include <string.h>
#include "usb_lib_usbip.h"
#include "usbip_server.h"
#include <pthread.h>

#define MIN(a, b) ((a < b) ? (a) : (b))

#define URST_IF   0x01
#define UERR_IF   0x02
#define SOF_IF    0x04
#define TRN_IF    0x08
#define IDLE_IF   0x10
#define RESUME_IF 0x20
#define STALL_IF  0x40
#define ALL_IF    0xFF

#define clr_int_flag(reg, mask) reg = mask;

#define __ep_QUEUE_SIZE 16

static usb_interrupt_handler_t usb_interrupt_handler;

static char U1EP[16];
static char U1ADDR = 0;
static int  SOFIF = 0;
static int  PKTDIS = 0;
static int  USBEN = 0;
static int  RESET = 0;

typedef struct {
	pthread_mutex_t lock;
	int front;
	int back;
	int queue[__ep_QUEUE_SIZE];
}  __ep_queue_t;

static __ep_queue_t __ep_queue;

void __ep_queue_init() {
	pthread_mutex_init(&__ep_queue.lock, NULL);
	__ep_queue.back = 0;
	__ep_queue.front = 0;
	memset(__ep_queue.queue, 0, sizeof(__ep_queue.queue));
}

int __is_event() {
	int r;
	pthread_mutex_lock(&__ep_queue.lock);
	r = __ep_queue.front != __ep_queue.back;
	pthread_mutex_unlock(&__ep_queue.lock);
	return r;
}

int __get_event() {
	int r, i;
	pthread_mutex_lock(&__ep_queue.lock);
	i = __ep_queue.front;
	__ep_queue.front = (__ep_queue.front + 1) % __ep_QUEUE_SIZE;
	r = __ep_queue.queue[i];
	pthread_mutex_unlock(&__ep_queue.lock);
	return r;
}

void __notify_ep(int ep) {
	pthread_mutex_lock(&__ep_queue.lock);
	int i = __ep_queue.back;
	__ep_queue.queue[i] = ep;
	__ep_queue.back = (__ep_queue.back + 1) % __ep_QUEUE_SIZE;
	pthread_mutex_unlock(&__ep_queue.lock);
}

void usb_init() {

//	PRINTF("&bdt: %X" NL, (unsigned short)__bdt_ptr);
//	PRINTF("bdt size: %d" NL, __bdt_size);

	// clear BDT
	memset((void*)__bdt_ptr, 0, __bdt_size);
	// TODO:
	clear_all_eps();
	// 
	pthread_mutex_init(&__ep_queue.lock, NULL);

	__usbip_server_init();
	
}

void usb_enable() {
	USBEN = 1;
} 

void usb_disable() {
	USBEN = 0;
}

void usb_reset() {
    PKTDIS = 0;
	U1ADDR = 0;
	SOFIF = 0;
	clear_all_eps();
	memset((char*)__bdt_ptr, 0, __bdt_size);
}

void usb_init_ep(int num, byte ep_type, ep_buf_dsc_t* ep)
{
	ep->in.stat = 0;
	ep->in.cnt = 0;
	ep->out.stat = 0;
	ep->out.cnt = 0;
	U1EP[num] = ep_type;
} 

void usb_ep_transf_start(ep_buf_dsc_t* ep, byte transfer_type, buf_ptr_t buffer, int size) {
    if ((transfer_type & 0x80) != 0) {
		ep->in.cnt = size & 0xFF;
		ep->in.addr = buffer;
		ep->in.stat = transfer_type | (size >> 8) | 0x80;
	} else {
		ep->out.cnt = size & 0xFF;
		ep->out.addr = buffer;
		ep->out.stat = transfer_type | (size >> 8) | 0x80;
	}
	PKTDIS = 0;
}
 
void clear_all_eps() {
	U1EP[0] = 0;
	U1EP[1] = 0;
	U1EP[2] = 0;
	U1EP[3] = 0;
	U1EP[4] = 0;
	U1EP[5] = 0;
	U1EP[6] = 0;
	U1EP[7] = 0;
	U1EP[8] = 0;
	U1EP[9] = 0;
	U1EP[10] = 0;
	U1EP[11] = 0;
	U1EP[12] = 0;
	U1EP[13] = 0;
	U1EP[14] = 0;
	U1EP[15] = 0;
}

void usb_set_address(byte addr) {
	U1ADDR = addr & 0xFF;
}


void copy_to_buffer(buf_ptr_t buf, byte* src, int size) {
    int i;
	for(i = 0; i < size; i++) {
	  *(buf++) = *(src++);
	} 
}

void copy_from_buffer(buf_ptr_t buf, byte* dest, int size) {
    int i;
	for(i = 0; i < size; i++) {
		*(dest++) = *(buf++);
	} 
}

int is_attached() {
	return 1; // TODO:
}

int is_powered() {
	return 1; // TODO:
}

int is_reset() {
	if (RESET != 0) {
		RESET = 0;
		return 1;
	}
	return 0;
}

int is_idle() {
	return 0;
}

int is_sof() { 
	if (SOFIF) {
		SOFIF = 0;
		return 1;
	}
	return 0;
}

int is_transfer_done() {
	return __is_event();	
}

int get_trn_status() {
	return __get_event();  
}			

int is_setup(int dir, ep_buf_dsc_t* ep) {
	if (dir != DIRECTION_OUT) {
		return 0;
	}
	return ((ep->out.stat >> 2) & 0x0F) == 0xd;
}

void usb_set_interrupt_handler(usb_interrupt_handler_t handler) {
	usb_interrupt_handler = handler;
}

void usb_interrupt_enable() {
}

void usb_interrupt_disable() {
}

int __usb_is_dev_attached() {
	return USBEN;
}

int __usb_transfer(int addr, int ep, int pid, byte* data, int size, int *rlen) {
	int sz;
	int epn = ep & 0x0F;
	int dir = ep & 0x80;
	int data0 = 0;
    ep_buf_dsc_t* bdt = (ep_buf_dsc_t*)__bdt_ptr;

	if (USBEN == 0 || epn >= __bdt_size || addr != U1ADDR || PKTDIS != 0) {
		return __RET_ERR;
	}

	switch (pid) {
		case __USB_RESET:
			RESET = 1;
			break;
		case __USB_PACKET_SOF:
			SOFIF = 1;
			break;
		case __USB_PACKET_SETUP:
			PKTDIS = 1;
			if ((bdt[epn].out.stat & 0x80) == 0) {
				return __RET_ERR;
			}
		case __USB_PACKET_DATA0:
			data0 = 0x40;
		case __USB_PACKET_DATA1:
			if ((ep & 0x80) == 0) {
				// out transfer
				if ((bdt[epn].out.stat & 0x80) == 0) {
					return __RET_NAK;
				}
				if (((bdt[epn].out.stat & 0x08) != 0) && (bdt[epn].out.stat & 0x40 == data0)) {
					return __RET_ERR;
				}
				sz = *((int*)(bdt[epn].out.addr - sizeof(int)));
				if (size > sz) {
					return __RET_ERR;
				}
				sz = MIN(bdt[epn].out.cnt, size);
				memcpy(bdt[epn].out.addr, data, sz);
				bdt[epn].out.stat = (bdt[epn].out.stat & 0xC3) | (pid << 2); 
				bdt[epn].out.cnt = sz;
				__notify_ep(ep);
				if (rlen != NULL) {
					*rlen = sz;
				}
				return __RET_ACK;
			} else {
				// in tranfer
				if ((bdt[epn].in.stat & 0x80) == 0) {
					return __RET_NAK;
				}
				sz = *((int*)(bdt[epn].in.addr - sizeof(int)));
				if (bdt[epn].in.cnt > sz) {
					return __RET_ERR;
				}
				sz = MIN(bdt[epn].in.cnt, size);
				memcpy(data, bdt[epn].in.addr, sz);
				bdt[epn].in.cnt = 0;
				bdt[epn].in.stat = (bdt[epn].in.stat & 0x40) | (pid << 2);					
				__notify_ep(ep);
				if (rlen != NULL) {
					*rlen = sz;
				}	
				return __RET_ACK;	
			}			
	}	
	return 0;
}
