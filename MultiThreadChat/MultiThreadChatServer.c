#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
void *do_chat(void *); //채팅 메세지를 보내는 함수
int pushClient(int); //새로운 클라이언트가 접속했을 때 클라이언트 정보 추가
int popClient(int); //클라이언트가 종료했을 때 클라이언트 정보 삭제
pthread_t thread;
pthread_mutex_t mutex;
#define MAX_CLIENT 3 
#define CHATDATA 1024
#define INVALID_SOCK -1
#define PORT 9024
//int    list_c[MAX_CLIENT];
typedef struct __clientData{
	int c_socket;
	char nickname[20];
} clientData;
clientData list_c[MAX_CLIENT];
char    escape[ ] = "exit";
char    greeting[ ] = "Welcome to chatting room\n";
char    CODE200[ ] = "Sorry No More Connection\n";
int main(int argc, char *argv[ ])
{
    int c_socket, s_socket;
    struct sockaddr_in s_addr, c_addr;
    int    len;
    int    i, j, n;
    int    res;
    if(pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Can not create mutex\n");
        return -1;
    }
    s_socket = socket(PF_INET, SOCK_STREAM, 0);
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    if(bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1) {
        printf("Can not Bind\n");
        return -1;
    }
    if(listen(s_socket, MAX_CLIENT) == -1) {
        printf("listen Fail\n");
        return -1;
    }
    for(i = 0; i < MAX_CLIENT; i++)
        list_c[i].c_socket = INVALID_SOCK;
    while(1) {
        len = sizeof(c_addr);
        c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
        res = pushClient(c_socket);
        if(res < 0) { //MAX_CLIENT만큼 이미 클라이언트가 접속해 있다면,
            write(c_socket, CODE200, strlen(CODE200));
            close(c_socket);
			printf("num =%d\n",res);
        } else {
            write(c_socket, greeting, strlen(greeting));
            //pthread_create with do_chat function.
            //스레드 생선
            int status = pthread_create(&thread,NULL,do_chat,(void *)&c_socket);
        }
    }
}
void *do_chat(void *arg)
{
    int c_socket = *((int *)arg);
    char chatData[CHATDATA];
	char *name;
    int i=0, n;
	while(i<MAX_CLIENT){
		if(list_c[i].c_socket == c_socket){
			name = list_c[i].nickname;
			break;
		}
		i++;
	}
	printf("now name = %s\n",name);
    while(1) {
        memset(chatData, 0, sizeof(chatData));
        if((n = read(c_socket, chatData, sizeof(chatData))) > 0) {
            //write chatData to all clients
			printf("data = %s\n",chatData);
        	if(strstr(chatData, escape)!=NULL) {
				popClient(c_socket);
           	 break;
        	}
			char *usrMsg = chatData;
			usrMsg = usrMsg+(strlen(name)+3);//"[닉네임] " 만큼 문자
			if(strncasecmp(usrMsg,"/w",2)==0){
				char namedesc[20];
				char *msg;
				char sendMsg[CHATDATA];
				memset(sendMsg,0,sizeof(sendMsg));
				strtok(usrMsg," ");// "/w"
				printf("usrMsg = %s\n",usrMsg);
				msg = strtok(NULL," ");
				if(msg != NULL){
					strcpy(namedesc,msg);
					printf("지금");
					printf("namedesc = %s\n",namedesc);
					msg = strtok(NULL," ");
					while(msg !=NULL){
						strcat(sendMsg,msg);
						strcat(sendMsg," ");
						msg = strtok(NULL," ");
					}
					i=0;
					while(i<MAX_CLIENT){
						if(strncasecmp(namedesc, list_c[i].nickname, strlen(namedesc)) == 0){
							sprintf(chatData,"[%s]'s whispher : %s\n",name,sendMsg);
							write(list_c[i].c_socket,chatData,strlen(chatData));
							break;	
						}
						else 
							i++;
					}
					if(i == MAX_CLIENT){
						sprintf(chatData,"no such user\n");
						write(c_socket,chatData,strlen(chatData));
					}
					
				}
				else {
					sprintf(chatData,"To use whispher : /w [username] [message]\n");
					write(c_socket,chatData,strlen(chatData));
				}
					
			}
			
			else{	
				i=0;
				while(i<MAX_CLIENT){
					write(list_c[i].c_socket,chatData,strlen(chatData));
					i++;
				}
			}
        }
	}
}


int pushClient(int c_socket) {
    //ADD c_socket to list_c array.
    //
    ///////////////////////////////
	int i=0;
	pthread_mutex_lock(&mutex);
	while(i<MAX_CLIENT){
		if(list_c[i].c_socket == INVALID_SOCK){ //find empty space from list_c
			list_c[i].c_socket = c_socket;
			pthread_mutex_unlock(&mutex);
			read(c_socket,list_c[i].nickname,sizeof(sizeof(char)*20));//receive nickname from client
			printf("name =%s\n",list_c[i].nickname);
			return i;
		} else
			i++;
	}
		pthread_mutex_unlock(&mutex);
		return -1;
    //return -1, if list_c is full.
    //return the index of list_c which c_socket is added.
}
int popClient(int c_socket)
{
    close(c_socket);
	int i=0;
    //REMOVE c_socket from list_c array.
	pthread_mutex_lock(&mutex);
	while(i<MAX_CLIENT){
		if(list_c[i].c_socket == c_socket){
			list_c[i].c_socket = INVALID_SOCK;
			memset((char *)&list_c[i].nickname,'\0',sizeof(char)*20);
			pthread_mutex_unlock(&mutex);
			return i;
		}
		else
			i++;
	}
	pthread_mutex_unlock(&mutex);
	return -1;
	

}
