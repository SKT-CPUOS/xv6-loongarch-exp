#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
	int id=sem_create(1);
	int pid = fork();
	int i;
	for(i=0;i<100000;i++){
		sem_p(id);
		sh_var_write(sh_var_read()+1);
		sem_v(id);
	}
	if(pid >0){
		wait(0);
		sem_free(id);
	}
	printf("sum=%d\n",sh_var_read());
	exit(0);
}