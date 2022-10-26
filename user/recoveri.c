#include "kernel/types.h" 
#include "kernel/stat.h" 
#include "user/user.h" 
#include "kernel/fcntl.h"

char buf[1024 * 256];

int main(int argc, char *argv[]) 
{
    if(argc < 2) 
    { 
        printf("format: savei filename temp\n"); exit(0);
    } 
    uint addrs[13]; 
    int fd;
    fd = open("temp", O_RDONLY); 
    read(fd, addrs, sizeof(addrs)); 
    close(fd);
    fd = open(argv[1], O_CREATE | O_RDWR); 
    int i ;
    printf("a:%d\n",sizeof(uint));
    for(i = 0; i < 12; i++) {
        printf("b:%x\n", addrs[i]);
    }

    for(i = 0; i < 12 && addrs[i] != 0; i++) {
        recoverb(addrs[i], buf, 0);
        write(fd, buf, 1024);
    }

    i = 0;

    if(addrs[12] != 0) {

        int ret = recoverb(addrs[12], buf, 1);
        
        write(fd, buf, ret);
    }
    // for(i = 0; i < 13 && addrs[i] != 0; i++) {
    //     recoverb(addrs[0], buf);
    //     write(fd, buf, 512);
    // } 
    close(fd); 
    exit(0);

}