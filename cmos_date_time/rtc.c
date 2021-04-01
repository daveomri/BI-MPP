/**
 * David Omrai
 * 10.6.2020
 */
#include <sys/io.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

//Addresses
#define INDEX_REGISTER  0x70
#define DATA_REGISTER   0x71

/**
 * Reads from cmos memory
 */
uint8_t readReg(unsigned short int address){
    //writes to the given i/o port
    outb(address, INDEX_REGISTER);
    //reads given i/o port
    return inb(DATA_REGISTER);
}

/**
 * Tests if data can be read
 */
int areDataValid(){
    return readReg(0x0a)&0x40;
}

/**
 * Decodes number from BCD
 */
int8_t decodeNum(int8_t num){
    return (num>>4)*10+(num&0x0f);
}

/**
 * This function prints binary representation of given
 * number
 */
void bin(unsigned n) 
{ 
    unsigned i; 
    for (i = 1 << 31; i > 0; i = i / 2) 
        (n & i)? printf("1"): printf("0"); 
} 

int main(int argc, char* argv[]){
    //to access V/V operations
    iopl(3);
    //wait for data to be valid
    while(areDataValid());

    //load data
    uint8_t sec, min, hour, day, weekDay, month, year;
    sec = decodeNum(readReg(0x00));
    min = decodeNum(readReg(0x02));
    hour = decodeNum(readReg(0x03));

    weekDay = decodeNum(readReg(0x06));
    day = decodeNum(readReg(0x07));
    month = decodeNum(readReg(0x08));
    year = decodeNum(readReg(0x09));
    
    //translate day of the week
    char days[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char* dayName = (char*)(days+(weekDay)%7);

    //translate month
    char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char* monthName = (char*)(months+(month-1)%12);

    //print result
    printf("%s %02d %s 20%d %02d:%02d:%02d\n", dayName, day,  monthName, year, hour, min, sec );

    return 0;
}
