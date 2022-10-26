#include "kernel/types.h" 
#include "kernel/stat.h" 
#include "user/user.h" 
#include "kernel/fcntl.h"


int main(int argc, char *argv[]) {

    if(argc < 2) { 
        printf("format: savei filename temp\n"); 
        exit(0); 
    }
    uint addrs[13]; 
    geti(argv[1], addrs);
    int fd = open("temp", O_CREATE | O_RDWR); 

    for(int i = 0; i < 13; i++) {
        printf("a:%x\n", addrs[i]);
    }
    write(fd, addrs, sizeof(addrs)); 
    close(fd); 
    exit(0);
}