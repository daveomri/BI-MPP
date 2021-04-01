#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <iostream>

#include <string>


#include <unistd.h>
#include <fcntl.h>

using namespace std;

int main(int argc, char* argv[]){
    //open
    int fd = open("/dev/mpp", O_RDWR);
    //write
    //string userInput;
    //cin >> userInput;
    write(fd, "uno a due a tre\0", 15);
    //write(fd, userInput.c_str(), userInput.length());
    int change_mess, reverse_mess;
    printf("Message 0/1: \n");
    cin >> change_mess;
    //ask
    if (change_mess)
        ioctl(fd, 100);
    printf("Reverse 0/1: \n");
    cin >> reverse_mess;
    if (reverse_mess)
        ioctl(fd, 101);
    
    //read
    char mess[25];
    read(fd, mess, 25);
    printf("Data received: %s\n", mess);
    // int i = 0;
    // for (i=0;i<25;i++)
    //     printf("%c ", mess[i]);
    //close
    close(fd);

    printf("Succesfully finished\n");
    return 0;
}