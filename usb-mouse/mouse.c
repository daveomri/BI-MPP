/**
 * Autor: David Omrai
 * 16.10.2020
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#define vid 0x0458
#define did 0x003a
#define wMaxPacketSize 0x0004

void free_aloc(struct libusb_device ** list, libusb_context *ctx){
    // dealokace seznamu souborů
    libusb_free_device_list(list, 1);
    // ukončení libusb
    libusb_exit(ctx);
}

int test_error(int r, struct libusb_device ** list, libusb_context *ctx){
    if (r < 0) {
        printf("ERROR: %s\n", libusb_strerror(r));
        free_aloc(list, ctx);
        return 1;
    }
    return 0;
}

void bin(uint32_t n, uint32_t size) 
{ 
    unsigned i; 
    for (i = 1 << (size-1); i > 0; i = i / 2) 
        (n & i)? printf("1"): printf("0"); 
    printf("\n");
} 

int main(int argc, char*argv[]){
    //Inicializace
    printf("Inicializace: start\n");
    const struct libusb_version *ver;
    libusb_context *ctx;

    libusb_init(&ctx);
    ver = libusb_get_version();
    printf("LIBUSB version %d.%d.%d.%d\n",
            ver->major, ver->minor, ver->micro, ver->nano);
    printf("Inicializace: done\n\n");

    //Detekce zařízení
    printf("Detekce zařízení: start\n");
    struct libusb_device **list;

    ssize_t count = libusb_get_device_list(NULL, &list); 
    printf("Number of devices: %ld\n", count);

    // procházejte seznam (pole), získejte device deskriptory a z nich vytiskněte VID:PID
    libusb_device* mouse;
    uint8_t dev_adrs;
    struct libusb_device_descriptor devdscr;

    for (int i = 0; i < count; i++){
        libusb_get_device_descriptor(list[i], &devdscr);
        dev_adrs = libusb_get_device_address(list[i]);
        printf("%04X:%04X (%d) \n", devdscr.idVendor, devdscr.idProduct, dev_adrs);
        // pokud se VID a PID rovná požadovanému zařízení, uchovejte si ukazatel na zařízení (list[i]). 
        if (devdscr.idVendor == vid && devdscr.idProduct == did){
            printf("Found it\n");
            mouse = list[i];
            break;
        }
    }
    printf("Detekce zařízení: done\n\n");
    
    //Vyčtení device deskriptoru
    printf("Vyčtení device deskriptoru: start\n");
    struct libusb_device_descriptor dev_dsc;
    if (test_error(libusb_get_device_descriptor(mouse, &dev_dsc), list, ctx)) return 1;
    printf("Vyčtení device deskriptoru: done\n\n");

    //Vyčtení konfiguračního deskriptoru
    printf("Vyčtení konfiguračního deskriptoru: start\n");
    struct libusb_config_descriptor *cfg_dsc;
    if (test_error(libusb_get_config_descriptor(mouse, 0, &cfg_dsc), list, ctx)) return 1;
    printf("Vyčtení konfiguračního deskriptoru: done\n\n");

    //Otevření zařízení
    printf("Otevření zařízení: start\n");
    libusb_device_handle* dh = libusb_open_device_with_vid_pid(ctx, devdscr.idVendor, devdscr.idProduct);
    if (dh == NULL) {
            printf("ERROR: %s\n", "cannot open the device");
            // zde dealokuj zdroje
            free_aloc(list, ctx);
            return 1;
    }
    printf("Otevření zařízení: done\n\n");

    //Odpojení ovladače jádra od interfacu
    printf("Odpojení ovladače jádra od interfacu: start\n");
    libusb_detach_kernel_driver (dh, 0);
    printf("Odpojení ovladače jádra od interfacu: done\n");

    //Výběr konfigurace a rozhraní
    printf("Výběr konfigurace a rozhraní: start\n");
    if(test_error(libusb_set_configuration(dh, cfg_dsc->bConfigurationValue), list, ctx)) return 1;
    if(test_error(libusb_claim_interface(dh, 0), list, ctx)) return 1;
    printf("Výběr konfigurace a rozhraní: done\n");

    //Čtení dat z koncového bodu typu přerušení (interrrupt)
    unsigned char data[1024];
    int actual_length = 0;
    //      D7   | D6   | D5   | D4   | D3 | D2    | D1   | D0  |
    uint8_t /*yover, xover, ysign, xsign, tag,*/ middle, right, left;//1.byte
    int     xnum,//x7,    x6,    x5,    x4,    x3,  x2,     x1,    x0,  //2.byte
            ynum,//y7,    y6,    y5,    y4,    y3,  y2,     y1,    y0,  //3.byte
            znum;//z7,    z6,    z5,    z4,    z3,  z2,     z1,    z0;  //4.byte
            //x0-7 move in x axis, y0-7 move in y axis, z0-7 wheel move
    
    while(1){
        if(test_error(libusb_interrupt_transfer(dh, 0x81, data, 4, &actual_length, 10000), list, ctx)) return 1;
        bin(data[0], 8);
        middle= data[0]>>2&0x1;
        right = data[0]>>1&0x1;
        left  = data[0]   &0x1;

        //Y UP    255..1, DOWN 1..255
        //X RIGHT 255..1, LEFT 1..255
        xnum = data[1] >= 128 ? data[1]-255 : data[1];
        ynum = data[2] >= 128 ? 255-data[2] : -data[2];
        znum = data[3] >= 128 ? data[3]-255 : data[3];

        printf("middle:%d right:%d left:%d\n", middle, right, left);
        printf("xnum: %d\n", xnum);
        printf("ynum: %d\n", ynum);
        printf("znum: %d\n", znum);
    }

    //Zakončení programu
    free_aloc(list, ctx);
    return 0;
}