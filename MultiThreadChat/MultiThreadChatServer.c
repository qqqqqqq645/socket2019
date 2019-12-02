#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

void *do_chat(void *); //채팅 메세지를 보내는 함수
int pushClient(int); //새로운 클라이언트가 접속했을 때 클라이언트 정보 추가
int popClient(int); //클라이언트가 종료했을 때 클라이언트 정보 삭제
int top; //클라이언트 스택 관리
pthread_t thread;
pthread_mutex_t mutex;
#define MAX_CLIENT 10 
#define CHATDATA 1024
#define INVALID_SOCK -1
#define PORT 9001
//int    list_c[MAX_CLIENT];
typedef struct __clientData{
	int c_socket;
	char nickname[20];
	int chatroom;
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
        } else {
            write(c_socket, greeting, strlen(greeting));
            //pthread_create with do_chat function.
            //스레드 생선
            //int status = pthread_create(&thread,NULL,do_chat,(void *)&c_socket);
            int status = pthread_create(&thread,NULL,do_chat,(void *)&list_c[res]);
        }
		printf("top = %d\n",top);
    }
}
void *do_chat(void *arg)
{
	//클라이언트 socket 정보 저장
	//int c_socket = *((int *)arg); //arg에서 int크기만
	//int c_socket = ((clientData *)arg)->c_socket; // arg 구조체크기 형변환
	int c_socket = (*(clientData *)arg).c_socket; //arg 구조체 형변환후역참조
	printf("c_socket add = %d\n",(int *) arg);
	char username [20];
	//strcpy(username,(char *)(arg+sizeof(int))); //nickname 저장
	strcpy(username,((clientData *)arg)->nickname);
	//int roomnum = *((int *)(arg+sizeof(int)+(sizeof(char)*20)));
	int roomnum = (*(clientData *)arg).chatroom;
    char chatData[CHATDATA];
	char chatalert [CHATDATA];
	char *usrMsg;
    int i=0, n,roomSelect=1;
	memset(chatalert,0,sizeof(chatalert));
	while(roomSelect){ //처음 입장시 채팅방 선택
        memset(chatData, 0, sizeof(chatData)); 
		strcpy(chatalert,"select chatroom (1~10) : ");
		write(c_socket, chatalert,strlen(chatalert));
    	if((n = read(c_socket, chatData, sizeof(chatData))) > 0) {
			//usrMsg = chatData;
			//usrMsg = usrMsg+(strlen(username)+3);//"[닉네임] " 만큼 문자열 포인터 이동
			usrMsg = strtok(chatData," ");
			if(((usrMsg = strtok(NULL," "))!=NULL)&& ((roomnum = atoi(usrMsg)))>0 
					&& roomnum<=10){

				//roomnum =  atoi(usrMsg);
				pthread_mutex_lock(&mutex);
				while(i<top){
					if(c_socket == list_c[i].c_socket){
						list_c[i].chatroom = roomnum;
						roomSelect=0;
						break;
					}
					i++;
				}
				pthread_mutex_unlock(&mutex);
			}
		}
	}
	sprintf(chatalert,"You are in chatroom %d\n",roomnum);
	write(c_socket, chatalert,strlen(chatalert));
    while(1) {
        memset(chatData, 0, sizeof(chatData)); 
		printf("%s top = %d\n",username,top);
        if((n = read(c_socket, chatData, sizeof(chatData))) > 0) {
            //write chatData to all clients
			printf("data = %s\n",chatData);
        	if(strstr(chatData, escape)!=NULL) { //escape의 문자 입력시 해당 스레드 종료
				i=0;
				sprintf(chatalert,"[%s] has left the room\n",username);
				while(i<top){
					if(roomnum == list_c[i].chatroom) //나갈때 방에 알림
						write(list_c[i].c_socket,chatalert,strlen(chatalert));
					i++;
				}
				popClient(c_socket);
           	 break;
        	}
			usrMsg = chatData;
			usrMsg = usrMsg+(strlen(username)+3);//"[닉네임] " 만큼 문자열 포인터 이동
			if(strncasecmp(usrMsg,"/who",4)==0){//방에 누가 있는지 검색
				i = 0;
				int roomMem=0;
					memset(chatData,0,sizeof(chatData));
					sprintf(chatData,"There are %d people in chat server\npeople in the chatroom [%d]\n",top,roomnum);
				while(i<top){
					if(roomnum == list_c[i].chatroom)
						strcat(chatData,list_c[i].nickname);
					if(strcmp(username,list_c[i].nickname)==0)
						strcat(chatData," (You)");
					strcat(chatData,"\n");
					i++;
				}
				write(c_socket,chatData,strlen(chatData));
			}
			else if(strncasecmp(usrMsg, "/c",2) == 0){// 채팅방 바꾸는 기능
				strtok(usrMsg," ");
				char *temp;
				if((temp = strtok(NULL," "))!=NULL){// "/c"
					i=0;
					int croom = atoi(temp);
					if(croom >0 && croom <=10){
						while(i<top){
							if(c_socket == list_c[i].c_socket)
								list_c[i].chatroom = croom;
								roomnum = croom;
							i++;			
						}
						sprintf(chatalert,"now you are in chatroom [%d]\n",croom);
						write(c_socket,chatalert,strlen(chatalert));
					}else{
						sprintf(chatalert,"/c [room number (1~10)]\n");
						write(c_socket,chatalert,strlen(chatalert));
					}
					
				}
				else{
					sprintf(chatalert,"/c [room number (1~10)]\n");
					write(c_socket,chatalert,strlen(chatalert));
				}
				
			}
			else if(strncasecmp(usrMsg,"/w",2)==0){ //귓속말 판별
				char namedesc[20]; //귓속말 대상 닉네임
				char *msg;
				char sendMsg[CHATDATA];
				memset(sendMsg,0,sizeof(sendMsg)); //보낼 메세지 문자열 초기화
				strtok(usrMsg," ");// "/w"
				printf("usrMsg = %s\n",usrMsg);
				msg = strtok(NULL," ");//닉네임 토큰
				if(msg != NULL){
					strcpy(namedesc,msg); //대상 닉네임 복사
					msg = strtok(NULL," ");//다음 토큰으로 이동
					while(msg !=NULL){ //메세지 재조립 
						strcat(sendMsg,msg);
						strcat(sendMsg," ");
						msg = strtok(NULL," "); 
					}
					i=0;
					while(i<top){//0부터 top까지 해당 닉네임이 있는지 탐색
						if(strncasecmp(namedesc, list_c[i].nickname, strlen(namedesc)) == 0){
							sprintf(chatData,"[%s]'s whispher : %s",username,sendMsg);
							write(list_c[i].c_socket,chatData,strlen(chatData));
							break;	
						}
						else 
							i++;
					}
					if(i == top){ //top까지 탐색했는데 없을 경우
						sprintf(chatData,"no such user\n");
						write(c_socket,chatData,strlen(chatData));
					}
					
				}
				else {
					sprintf(chatData,"To use whispher : /w [username] [message]\n");
					write(c_socket,chatData,strlen(chatData));
				}
					
			}
			else { // 아무 명령어 없을 경우 전체 대화
				i=0;
				while(i<top){
					if(roomnum == list_c[i].chatroom)
					write(list_c[i].c_socket,chatData,strlen(chatData));
					
					i++;
						printf("err");
				}
			}
        }
	}
}


int pushClient(int c_socket) {
    //ADD c_socket to list_c array.
	int i=0;
	char namebuf[20];
	char sendbuf [CHATDATA];
	//pthread_mutex_lock(&mutex);
	if(top<MAX_CLIENT){
			list_c[top].c_socket = c_socket;
			//read(c_socket,list_c[top].nickname,sizeof(sizeof(char)*20));//receive nickname from client
			 //이름 중복검사후 중복되면 연결종료
			 	memset(namebuf,0,sizeof(namebuf));
				read(c_socket,namebuf,sizeof(char)*20);
				while(i<=top){
					printf("loop entered\n");
					if(strcmp(namebuf,list_c[i].nickname)==0){
						sprintf(sendbuf,"[%s] is already exists!!\n",namebuf);
						write(c_socket,sendbuf,strlen(sendbuf));
						return -1;
					}
					i++;
				}
			
			strcpy(list_c[top].nickname,namebuf);
			/*srand(time(NULL));
			list_c[top].chatroom = (int)(rand()%10)+1;//1에서 10사이의 무작위 대화방 입장*/
			//pthread_mutex_unlock(&mutex);
			printf("top in pushC = %d\n",top);
			printf("list add[toop] = %d\n", (int *)&list_c[top]);
			printf("sockadd = %d\n",(int *)&list_c[top].c_socket);
			printf("nameadd = %d\n",(int *)&list_c[top].nickname);
			return top++;
		} 
	else
	
		//pthread_mutex_unlock(&mutex);
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
	while(i<top-1){
		if(list_c[i].c_socket == c_socket){
			int j = i;
			while(j<top-1){
				list_c[j].c_socket = list_c[j+1].c_socket;
				strcpy(list_c[j].nickname,list_c[j+1].nickname);
				j++;
			}
			break;
		}
		i++;
	}
			printf("top in popC before before mutex = %d\n",top);
	list_c[--top].c_socket = INVALID_SOCK;
	memset(list_c[top].nickname,0,strlen(list_c[top].nickname));
			printf("top in popC before mutex = %d\n",top);
	pthread_mutex_unlock(&mutex);
			printf("top in popC = %d\n",top);
	return top;
	

}
