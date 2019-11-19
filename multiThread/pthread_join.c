#include <stdio.h>
#include <pthread.h>

void *do_sum(void *);// 스레드가 수행할 함수
//1부터 인자로 받은 수까지의 합을 출력
int main(){
	pthread_t thread_id[3];
	int status[3];
	int ar[3];
	ar[0] = 4;//1번 스레드 인자
	ar[1] = 5;//2번 "
	ar[2] = 6;//3번 "
	printf("ar = %d\n",ar[0]);
	int i=0;
	for(i=0;i<3;i++){
//		printf("ar[%d] = %d\n",i,ar[i]);
//		printf("void)&ar[%d] = %d\n ",i,&ar[i]);
		status[i] = pthread_create(&thread_id[i], NULL , do_sum,(void *) &ar[i]);
		pthread_join(thread_id[i],(void **)&status[i]);
	}	
	printf("Main thread END\n");

}
void *do_sum(void *argv){
	int sum=0;
	int i=0;
	for(i=1;i<*(int *)argv;i++){
		sum+=i;
		printf("[%d] - ADD %d \n",sum,i);
	}
	printf("1부터 [%d] 까지의 합은 [%d] 입니다.\n",*(int *)argv,sum);
	
	//printf("argv = %d\n",*((int* )argv));
}
