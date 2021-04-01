/**
 * David Omrai
 * 10.6.2020
 */

#include <sys/io.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

//addresses
#define INDEX_REGISTER 0xCF8
#define DATA_REGISTER 0xCFC

uint32_t pci_cfg_read(uint32_t cfg_addr){
    return inl(cfg_addr);
}

void pci_cfg_write(uint32_t cfg_addr, uint32_t val){
    outl(val, cfg_addr);
}


void bin(unsigned n) 
{ 
    unsigned i; 
    for (i = 1 << 31; i > 0; i = i / 2) 
        (n & i)? printf("1"): printf("0"); 
} 

int main(int argc, char* argv[]){
    //to access I/O operations
    iopl(3);

    //|sbernice|zarizeni|funkce|register(0)
    uint32_t address;
    uint32_t result;

    uint16_t higherHalf, lowerHalf;

    for (uint32_t bus = 0; bus<=0xff; bus++)
        for (uint32_t dev = 0; dev<=0x1f; dev++)
            for(uint32_t fun = 0; fun<=0x07; fun++){
                address = inl(INDEX_REGISTER)&0xff000003;
                address += fun<<8;
                address += dev<<11;
                address += bus<<16;

                //read pci
                pci_cfg_write(INDEX_REGISTER, address);
                result = pci_cfg_read(DATA_REGISTER);

                if ( result!=0xffffffff ){
                    // printf("bus number: %d\n", bus);
                    printf("device number: %d\n", dev);
                    // printf("function number: %d\n", fun);
                    // printf("register number: 0\n");
                    higherHalf = result>>16;
                    lowerHalf  = result&0xffff;
                    printf("%04x:%04x\n", lowerHalf, higherHalf);
                    //printf("--------------------\n");
                }
            }
    return 0;
}
