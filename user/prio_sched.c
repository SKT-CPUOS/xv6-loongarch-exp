#include"kernel/types.h"
#include"kernel/stat.h"
#include"user/user.h"

int
main(int argc, char * argv[]) 
{
    int pid;
    printf("This is a demo for prio-schedule!\n");
    pid = getpid();
    chpri(pid,19);  //系统默认优先级是10

    int i = 0;
    
    pid = fork();
    if(pid == 0)            //子进程
    {
        chpri(getpid(), 5);
        i = 1;
        while(i <= 10000) 
        {
            if(i / 1000 == 0) 
            {
                printf("p2 is running\n");
            }
            i++;
        }
        printf("p2 sleeping\n");
        sleep(100);
        i=1;
        while(i <= 1000000)
        {
            if(i/100000 == 0)
            {
                printf("p2 is running again\n");
            } 
            i++;
        }
        printf("p2 finished\n");
    }else               //父进程
    {
        i = 1;
        while (i > 0)
        {
            if(i/1000000 == 0)
                printf("p1 is running\n");
        }
        
    }
    exit(0);
}