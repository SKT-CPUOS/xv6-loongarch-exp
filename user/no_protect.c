#include"kernel/types.h"
#include"kernel/stat.h"
#include"user/user.h"

int
main(int argc, char* argv[]) {
    char* p = (char *)0x0b30;
    for(int i = 0x0000; i < 0x08; i++) {
        *(p + i) = '*';
    }
    printf("This string shouldn't be modified\n");
    exit(0);
}