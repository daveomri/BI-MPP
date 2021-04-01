/*************************************************************
 * COURSE WARE ver. 3.0                                                                                                                                                                        *
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

#ifndef __USB_LIB_USBIP
#define __USB_LIB_USBIP

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

typedef byte* buf_ptr_t;

typedef void (*usb_interrupt_handler_t)(void);

extern const void* __bdt_ptr;

extern const int __bdt_size;
  
#define DECLARE_EP_BEGIN struct { 
#define DECLARE_EP_END  } __bdt __attribute__ ((aligned (512))); \
		const void* __bdt_ptr = (void*)&__bdt;  \
		const int __bdt_size = sizeof(__bdt);
#define DECLARE_EP(name) ep_buf_dsc_t name;
#define EP(ep) ((ep_buf_dsc_t*)(&__bdt.ep))

#define DECLARE_BUFFER_BEGIN
#define DECLARE_BUFFER_END
#define DECLARE_BUFFER(name, sz) struct __attribute__((packed)) { int size; byte data[sz];  } name ## _struct = { .size = sz }; \
                byte* name = name ## _struct.data;
   
// Features
#define ENDPOINT_HALT 0
#define DEVICE_REMOTE_WAKEUP 1
#define TEST_MODE 2

#define EP_SETUP_INOUT     0x0D
#define EP_OUT             0x09
#define EP_IN              0x05
#define EP_OUT_ISO         0x08
#define EP_IN_ISO          0x04
        
#define USB_TRN_NONE       0x00   // 0x00 
#define USB_TRN_SETUP      0x08   // 0x0C
#define USB_TRN_DATA0_OUT  0x08   // 0x08
#define USB_TRN_DATA0_IN   0x88   // 0x88
#define USB_TRN_DATA1_OUT  0x48   // 0x48
#define USB_TRN_DATA1_IN   0xC8   // 0xC8

#define GET_STATUS         0
#define CLEAR_FEATURE      1
#define SET_FEATURE        3
#define SET_ADDRESS        5
#define GET_DESCRIPTOR     6
#define SET_DESCRIPTOR     7
#define GET_CONFIGURATION  8
#define SET_CONFIGURATION  9
#define GET_INTERFACE      10
#define SET_INTERFACE      11
#define SYNCH_FRAME        12

#define HOST_TO_DEV        (0 << 7)
#define DEV_TO_HOST        (1 << 7)
#define STANDARD           (0 << 5)
#define CLASS              (1 << 5)
#define VENDOR             (2 << 5)
#define DEVICE             (0 << 5)
#define INTERFACE          1
#define ENDPOINT           2
#define OTHER              3

#define DSC_DEVICE               1
#define DSC_CONFIGURATION        2
#define DSC_STRING               3
#define DSC_INTERFACE            4
#define DSC_ENDPOINT             5
#define DSC_DEVICE_QUALIFIER     6
#define DSC_OSC                  7
#define DSC_INTERFACE_POWER      8

#define CS_INTERFACE            0x24
#define CS_ENDPOINT             0x25

#define DSC_SUBTYPE_CDC_HEADER              0x00
#define DSC_SUBTYPE_CDC_CALL_MANAGEMENT_FD  0x01
#define DSC_SUBTYPE_CDC_ACM_FD              0x02
#define DSC_SUBTYPE_CDC_UNION_FD            0x06

#define DIRECTION_IN             1
#define DIRECTION_OUT            0

typedef struct __attribute__((packed)) usb_device_req {	
	byte bmRequestType;	        
	byte bRequest;		         
	word wValue;		        
	word wIndex;		        
	word wLength;		        
} usb_device_req_t;

typedef struct __attribute__((packed)) device_descr	    
{			
    byte bLength;		        
    byte bDescriptorType;	    
    word bcdUSB;		                                     
    byte bDeviceClass;
    byte bDeviceSubClass;
    byte bDeviceProtocol;
    byte bMaxPacketSize;
    word idVendor;		      
    word idProduct;		          
    word bcdDevice;		      
    byte iManufacturer;		    
    byte iProduct;		        
    byte iSerialNumber;		    
    byte bNumConfigurations;     	
} device_descr_t;

typedef struct __attribute__((packed)) config_descriptor
{   byte bLength;		           
    byte bDescriptorType;	        
    word wTotalLength;		       
    byte bNumInterfaces;		       
    byte bConfigurationValue;	   
    byte iConfiguration;	       
    byte bmAttributes;		       
    byte bMaxPower;		           
} config_descriptor_t;

typedef struct __attribute__((packed)) interf_descriptor   
{
    byte bLength;		           
    byte bDescriptorType;	       
    byte bInterfaceNumber;  	   
    byte bAlternateSetting;	                                          
    byte bNumEndpoints;		      
    byte bInterfaceClass;	      
    byte bInterfaceSubClass;	  
    byte bInterfaceProtocol;	  
    byte iInterface;		      
} interf_descriptor_t;

typedef struct __attribute__((packed)) endpoint_descriptor 
{	byte bLength;			       
    byte bDescriptorType;		   
    byte bEndpointAddress;		   
    byte bmAttributes;			   
    word wMaxPacketSize;		  
    byte bInterval;			       
} endpoint_descriptor_t;

typedef struct __attribute__((packed)) string_descriptor   
{   byte bLength;			       
	byte bDescriptorType;		  
 	byte bString[1];		       
} string_descriptor_t;

typedef struct __attribute__((packed)) header_funct_descriptor
{
    byte bFunctionalLength;
    byte bDescriptorType;
    byte bDescriptorSubType;
    word bcdCDC;
} header_funct_descriptor_t;

typedef struct __attribute__((packed)) cdc_acm_funct_descriptor
{
    byte bFunctionalLength;
    byte bDescriptorType;
    byte bDescriptorSubType;
    byte bmCapabilities;
} cdc_acm_funct_descriptor_t;

typedef struct __attribute__((packed)) cdc_union_funct_descriptor
{
    byte bFunctionalLength;
    byte bDescriptorType;
    byte bDescriptorSubType;
    byte bmMasterInterface;
    byte bSlaveInterface0;
} cdc_union_funct_descriptor_t;

typedef struct __attribute__((packed)) cdc_call_mangement_funct_descriptor
{
    byte bFunctionalLength;
    byte bDescriptorType;
    byte bDescriptorSubType;
    byte bmCapabilities;
    byte bDataInterface;
} cdc_call_management_funct_descriptor_t;

typedef struct __attribute__((packed)) bdf_entry {
		byte cnt; 
		byte stat;
		buf_ptr_t addr; 
} bdf_entry_t ;   

	
typedef struct __attribute__((packed)) ep_buf_dsc          
{	
	bdf_entry_t out;
	bdf_entry_t in;	 
} ep_buf_dsc_t;

extern void usb_init();
extern void usb_init_ep(int num, byte ep_type, ep_buf_dsc_t* ep);
extern void usb_ep_transf_start(ep_buf_dsc_t* ep, byte transf_type, buf_ptr_t buffer, int size);
extern void clear_all_eps(void);
extern void usb_reset(void);
extern void usb_enable(void);
extern void usb_disable(void);
extern void usb_set_address(byte addr);
extern int is_attached();
extern int is_powered();
extern int is_reset();
extern int is_idle();
extern int is_sof();
extern int is_transfer_done();
extern int get_trn_status();
extern int extract_ep_num(int status);
extern int extract_ep_dsc(int status);
extern int is_setup(int dir, ep_buf_dsc_t* ep);
extern void copy_to_buffer(buf_ptr_t buf, byte* src, int size);
extern void copy_from_buffer(buf_ptr_t buf, byte* dest, int size);
extern int get_ep_direction(ep_buf_dsc_t* ep);
extern void usb_interrupt_enable();
extern void usb_interrupt_disable();
extern void usb_set_interrupt_handler(usb_interrupt_handler_t handler);

#define __USB_PACKET_SETUP        0x0D
#define __USB_PACKET_DATA0        0x03
#define __USB_PACKET_DATA1        0x0B
#define __USB_PACKET_SOF          0x05
#define __USB_PACKET_ACK          0x02
#define __USB_PACKET_STALL        0x0E
#define __USB_RESET               0xFF

#define __RET_ACK           0
#define __RET_NAK          -1        
#define __RET_ERR          -2
#define __RET_STALL        -3 

int  __usb_is_dev_attached();
void __usb_reset();
int __usb_transfer(int addr, int ep, int type, byte* data, int size, int *rlen);

#endif

