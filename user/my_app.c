#include"kernel/types.h"
#include"kernel/stat.h"
#include"user/user.h"

int
main(int argc, char * argv[])
{
    int a;
    printf("This is my app!\n");
    a = fork();
    a = fork();

    while(1) {
        a++;
    }

    exit(0);
}