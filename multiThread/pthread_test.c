/*
 * 메인스레드가 종료되면 자식스레드도종료
 */




#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
//컴파일할때 pthread 라이브러리 추가
void *func_thread(void *);//스레드가 수행할 함수
//void * :최상위 형태 모든 타입으로 형변환 가능

int main(){
	int status;
	pthread_t thread_id;
	pthread_create(&thread_id, NULL, func_thread, NULL);
	
	
	printf("thred %x is created\n", thread_id);
	
	pthread_join(thread_id, (void **) &status);	


	printf("main thread END\n");
	return 0;
}
void *func_thread(void * argv){
	int i=0;
	while(i<=10){
		printf("thread %dth executing...\n",i++);
	}
}
