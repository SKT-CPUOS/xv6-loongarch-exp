#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
mem(void){
    bstat();

    // char *a1 [500];
    // char *a2 [500];
    // int pid = fork();
    // bstat();

    // if(pid == 0){
    //     printf("father\n");
    //     for(int i = 1; i <= 4;i ++){
    //         char *s = (char*)sbrk(4096);//纯分配，不访问是不会分配物理空间的
    //         s[1] = 1;
    //         bstat();
    //         // a1[i] = s; //记录页帧首地址
    //         // a1[i][1] = 101;
            
    //         // printf(1,"procid %d ;father addr %p \n",getpid() ,a1[i][1]);
    //         // bstat();
            
    //     }
        
    //     // for(int i = 1; i <= 345;i ++){
    //     //     a1[i][1] = 100; //开始分配345个页帧
    //     // }   
    //     //循环访问最后2个 即344 345
    //     // while(1){
    //     //     printf(1,"\n---%s---\n","father");
    //     //     printf(1,"%d : %d \n",344,a1[344][1]);
    //     //     printf(1,"%d : %d \n",345,a1[345][1]);
    //     //     sleep(100 * 5);
    //     //     printf(1,"\n----------\n");
    //     // }
        
    // }else if(pid > 0){
    //     printf("son\n");
    //     for(int i = 1; i <= 4;i ++){
    //         char *s = (char*)sbrk(4096);//纯分配，不访问是不会分配物理空间的
    //         s[1] = 2;
    //         bstat();
    //         // a2[i] = s;
    //         // a2[i][1] = 100;
            
    //         // printf(1,"procid %d ;son addr %p \n",getpid() ,a2[i][1]);
    //         // bstat();
            
    //     }
    //     // for(int i =1 ; i <= 347; i ++){
    //     //     a2[i][1] = 1;
    //     // }
    //     // for(int i = 1; i <= 347;i ++){
    //     //     a2[i][1] = 100; //开始分配347个页帧
    //     // }
    //     //循环访问最后4个 即344 345 346 347
    //     // while(1){
    //     //     printf(1,"\n---%s---\n","son");
    //     //     printf(1,"%d : %d \n",344,a2[344][1]);
    //     //     printf(1,"%d : %d \n",345,a2[345][1]);
    //     //     sleep(100 * 5);
    //     //     printf(1,"%d : %d \n",346,a2[346][1]);
    //     //     printf(1,"%d : %d \n",347,a2[347][1]);
    //     //     printf(1,"\n----------\n");
    //     // }
        
    // }else{
    //     printf("error\n");
    // }


    // bstat();

    char *addrTable[11];

    for(int i = 0; i < 4; i++) {
        addrTable[i] = sbrk(4096);
    }

    // bstat();

    printf("----------------------------\n");

    for(int i = 0; i < 4; i++) {
        addrTable[i][1] = 'a' + i; 
        // bstat();
    }

    printf("----------------------------\n");

    printf("访问第四个页，其内容是%c, 不发生缺页异常\n", addrTable[3][1]);

    printf("下面展示缺页异常的交换功能：\n");

    for(int i = 0, cnt = 1; i < 4;i ++, cnt++) {
        bstat();
        printf("第%d个页的内容是：%c\n", cnt, addrTable[i][1]);
        sleep(3);
    }

}


int
main(void)
{
    bstat();
    char *s = 0;

    //先把页帧数量减少，便于观察实验
    for(int i = 1 ; i <= 238; i++) {
        s = sbrk(4096);
        s[1] = 1;
        
    }
        
    bstat();
    printf("------------------\n");
    mem();
    exit(0);
    return 0;
}