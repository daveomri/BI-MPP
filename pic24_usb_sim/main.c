#include <stdio.h>
#include "usb_lib_usbip.h"
//#include "usb_lib.h"

#define MIN(a,b) ((a < b) ? (a) : (b))
#define MAX(a,b) ((a > b) ? (a) : (b))

// Délky bufferů
#define EP0_OUT_BUF_SIZE 64  
#define EP0_IN_BUF_SIZE 64

// Deklarace endpointů
DECLARE_EP_BEGIN
	DECLARE_EP(ep0);   // IN/OUT Control EP
DECLARE_EP_END

// Deklarace bufferů
DECLARE_BUFFER_BEGIN
	DECLARE_BUFFER(ep0_buf_out, EP0_OUT_BUF_SIZE);
	DECLARE_BUFFER(ep0_buf_in, EP0_IN_BUF_SIZE);
DECLARE_BUFFER_END


#define DETACHED 0
#define ATTACHED 1
#define POWERED  2        
#define DEFAULT  3

int device_state = DETACHED;

const device_descr_t devdsc = {//__attribute__((space(auto_psv))) 
	sizeof(device_descr_t),
    1,
	0x0110,
    0,
    0,
    0,
    64,
    0x1111,
    0x2222,
    0x0100,
    0,
    0, 
    0,
    1
};


const struct config {
	config_descriptor_t config_descr;
	interf_descriptor_t interf_descr;
	endpoint_descriptor_t endp_descr1;
} CONFIG = {//__attribute__((space(auto_psv))) = 
	{ 	
		sizeof(config_descriptor_t),
		2,               // bDescriptorType
		sizeof(CONFIG),  
		1,               // bNumInterfaces
		0x01,            // bConfigurationValue
		0,               // iConfiguration
		0x80,            // bmAttributes
		50               // bMaxPower
	},
	{ 
		sizeof(interf_descriptor_t),
		4,               // bDescriptorType
		0,               // bInterfaceNumber
		0x0,             // bAlternateSetting 
		3,               // bNumEndpoints
		0xFF,            // bInterfaceClass
		0x0,             // bInterfaceSubClass 
		0x0,             // bInterfaceProtocol 
		0                // iInterface 
	},
	{
		sizeof(endpoint_descriptor_t),
		5,               // bDescriptorType 
		0x81,            // bEndpointAddress
		3,               // bmAttributes
		10,              // wMaxPacketSize
		1                // bInterval
	}
};

#define EP0_STATE_SETUP 0
#define EP0_STATE_DATA1_IN 1
#define EP0_STATE_ACK   2

int ep0_state = 0;

//realizace ridicich prenosu na endpoint
void process_control_transfer(int ep){
	int setup;
	usb_device_req_t req;

	printf("Process control transfer\n");
	setup = is_setup(ep&0x80, EP(ep0));

	if (setup){
		copy_from_buffer(ep0_buf_out, (byte*)&req, sizeof(usb_device_req_t));
		printf("RequestType: 0x%02X\n", req.bmRequestType);
		printf("bRequest: 0x%02X\n", req.bRequest);
		printf("wValue: 0x%04X\n", req.wValue);
		printf("wIndex: 0x%04X\n", req.wIndex);
		printf("wLength: 0x%04X\n", req.wLength);

		switch(req.bRequest) {
			case 6:
				switch((req.wValue >> 8)&0xFF) {
					case 1:
						copy_to_buffer(ep0_buf_in, (byte*)&devdsc, sizeof(devdsc));
						usb_ep_transf_start(EP(ep0), USB_TRN_DATA1_IN, ep0_buf_in, MIN(sizeof(devdsc), req.wLength));
						ep0_state = EP0_STATE_DATA1_IN;
						break; 
				} //setaddress pro usb z lib_usb po potvrzeni set_address
				break;
		}
		return;
	}
	printf("non setup transfer\n");

	switch (ep0_state){
		case EP0_STATE_DATA1_IN:
			usb_ep_transf_start(EP(ep0), USB_TRN_DATA1_OUT, ep0_buf_out, EP0_OUT_BUF_SIZE);
			ep0_state = EP0_STATE_ACK;
			break;
			//potvrzeni pripojeni od pc
		case EP0_STATE_ACK:
			usb_ep_transf_start(EP(ep0), USB_TRN_SETUP, ep0_buf_out, EP0_OUT_BUF_SIZE);
			ep0_state = EP0_STATE_SETUP;
			break;
	}
}
//rizeni na vsechny ostatni
void process_ep_transfer(int ep){
	printf("Process ep transfer\n");
}


int main(void) {
	printf("Hello, starting the program\n");

	// inicializujte USB podsystém
	usb_init();
	// vyčkejte dokud není zařízení připojeno k PC (attached)
	while (!is_attached());
	// zapněte USB podsystém
	usb_enable();
	// indikujte stav attached	
	device_state = ATTACHED;
	// počkejte dokud se USB subsystém nedostane do stavu powered
	while (!is_powered());
	// indikujte stav powered
	device_state = POWERED;


	while (1) {
       	// testujte USB reset 
		if (is_reset()) {
			printf("USB Reset\n");
			usb_reset();
			// inicializujte endpoint 0 (případně další endpointy)
			usb_init_ep(0, EP_SETUP_INOUT, EP(ep0));//               | omezen velikosti bufferu
			usb_ep_transf_start(EP(ep0), USB_TRN_SETUP, ep0_buf_out, EP0_OUT_BUF_SIZE);

			// nastartujte přenos přes endpoint 0 (doplňte)
			// po resetu přejděte do stavu DEFAULT  
			device_state = DEFAULT;
		}	

		// testujte IDLE (režim snížené spotřeby)
		if (is_idle()) {
			// reportujte do logu
			continue;
		}
		//start of frame, kazdou milisekundu
		if (is_sof()) {
			// reportujte do logu
			continue;	
		}
		
		// testujte na dokončení přenosu
		if (is_transfer_done()) {
			printf("transfer_done\n");
			// zjistěte číslo koncového bodu, kde došlo k dokončení přenosu
			
			int ep_num = get_trn_status();
			
			// detekujte řídící přenos (control transfer)
			if (ep_num == 0 || ep_num == 0x80) {
				// volejte funkci, kde obsloužíte řídící přenosy 
				process_control_transfer(ep_num);
				continue;
			}
			// zpracujte přenosy na ostatní koncové body 
			//process_ep_transfer(ep_num); 	
			continue;
		}

		// log_main_loop(); logovani ve vypisech, tady pres printf		
	}
	return 0;
}
