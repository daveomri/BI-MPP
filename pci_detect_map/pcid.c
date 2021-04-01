/**
 * David Omrai
 * 8.10.2020
 */

#include <sys/io.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

//power
uint32_t power(int base, int power){
    uint32_t tmp = 1;
    for (int i = 0; i < power; i+=1)
        tmp*=base;
    return tmp;
}

//addresses
#define INDEX_REGISTER  0xCF8
#define DATA_REGISTER   0xCFC

uint32_t pci_cfg_read(uint32_t cfg_addr){
    return inl(cfg_addr);
}

void pci_cfg_write(uint32_t cfg_addr, uint32_t val){
    outl(val, cfg_addr);
}

uint32_t construct_address(uint32_t bus, uint32_t dev, uint32_t fun){
    uint32_t address = inl(INDEX_REGISTER)&0xff000003;
    address += fun<<8;
    address += dev<<11;
    address += bus<<16;

    return address;
}

uint32_t read_reg(uint32_t address){
    pci_cfg_write(INDEX_REGISTER, address);
    return pci_cfg_read(DATA_REGISTER);
}

void write_reg(uint32_t address, uint32_t val){
    pci_cfg_write(INDEX_REGISTER, address);
    pci_cfg_write(DATA_REGISTER, val);
}

//(2)
//Zaměřte se na BAR v konfiguračním prostoru, 
//který mapuje registry druhého disku do V/V prostoru.
uint32_t find_bar(uint32_t address){
    uint32_t tmp;
    for (uint16_t i = 0x10; i <= 0x30; i+=0x04){
        tmp = read_reg(address+i);
        printf("l:%08x\n", tmp);
        if (tmp&0x00000001 == 0x00000001)
            return address+i;
    }
    return 0xffffffff;
}


void bin(unsigned n) 
{ 
    unsigned i; 
    for (i = 1 << 31; i > 0; i = i / 2) 
        (n & i)? printf("1"): printf("0"); 
} 


//(3)
//Zjistěte, kolik V/V registrů tento BAR mapuje.
uint32_t count_bar_reg(uint32_t address){
    uint32_t tmp;
    write_reg(address, 0xffffffff);
    tmp = read_reg(address)&0xfffffffc;

    uint32_t count=0;
    while ((tmp&0x1)==0){
        count+=1;
        tmp=tmp>>1;
    }
    return power(2, count);
}

//(1)
//Napište funkci, 
//která vyhledá PCI zařízení 
//s požadovaným VID a PID 
//(prohledávejte i funkce zařízení) 
//a vrátí bázovou adresu v konfiguračním 
//adresním prostoru.
uint32_t find_given_pci(uint16_t vid, uint16_t pid){
    uint32_t result;
    uint32_t tmp;

    uint16_t tmpVid, tmpPid;
    for (uint32_t bus = 0; bus<=0xff; bus++){
        for (uint32_t dev = 0; dev<=0x1f; dev++){
            for(uint32_t fun = 0; fun<=0x07; fun++){
                tmp = read_reg(construct_address(bus, dev, fun));

                if (tmp == 0xffffffff) continue;

                tmpVid = tmp & 0xffff;
                tmpPid = tmp >> 16;

                if (tmpVid == vid && tmpPid == pid){
                    result = construct_address(bus, dev, fun);
                    printf("Vendor id: %d\n", tmpVid);
                    printf("Device id: %d\n", tmpPid);
                    return result;
                    //printf("::%08x\n", read_dev_reg(bus, dev, fun, 0x10));
                }
            }
        }
    }
    return result;
}



int main(int argc, char* argv[]){
    //to access I/O operations
    iopl(3);

    uint32_t result;

    //ethernet device
    uint16_t vid = 0x8086;
    uint16_t pid = 0x100f;

    result = find_given_pci(vid, pid);

    uint32_t bar = find_bar(result);
    

    printf("Testovani zapisu do base registru, a to hodnotu 0x5a\n");
    //test input
    uint32_t base = read_reg(bar);
    base = (base >> 2)<<2;
    pci_cfg_write(base, 0x5a);
    printf("reg read: %08x\n", pci_cfg_read(base));

    printf("Testovani kolik v/v registru tento bar register mapuje\n");
    printf("mapuje: %d\n", count_bar_reg(bar));
    
    //(4)
    //Příkazem cat /proc/ioports zobrazte mapu V/V prostoru
    //a vyhledejte volnou oblast.

    //(5)
    //Ověřte přítomnost registrů disku na původní adrese,
    //změnou hodnoty v BAR registru přemapujte registry
    //řadiče disku ve V/V prostoru a ověřte přítomnost
    //registrů na nové adrese.
    uint32_t new_base;
    uint32_t new_value = 0x0d00;
    printf("setting new base: %08x\n", new_value);
    
    write_reg(bar, new_value);

    new_base = read_reg(bar);
    new_base = (new_base>>1)<<1;

    printf("new base: %08x\n", new_base);

    printf("Testovani zapisu do noveho base registru, 0x5a\n");
    pci_cfg_write(new_base, 0x5a);
    printf("new bar data: %08x\n", pci_cfg_read(new_base));


    //return all back
    printf("Navraceni puvodniho base\n");
    write_reg(bar, base);
    printf("old bar data: %08x\n", pci_cfg_read(base));

    //the end
    return 0;
}
