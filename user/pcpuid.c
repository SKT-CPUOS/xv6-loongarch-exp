#include"kernel/types.h"
#include"kernel/stat.h"
#include"user/user.h"

int
main(int argc, char * argv[])
{
    printf("My CPU id is:%d\n", getcpuid());

    exit(0);
}