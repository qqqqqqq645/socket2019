#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

#define BUFSIZE 100
#define PORT 10000

char buffer[BUFSIZE] = "Hi I'm server\n";
char rcvBuffer[BUFSIZE];

int main(){
	int c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	int len;
	int n;

	// 1. 서버 소켓 생성
	//서버 소켓 = 클라이언트의 접속 요청을 처리(허용)해 주기 위한 소켓
	s_socket = socket(PF_INET, SOCK_STREAM, 0); //TCP/IP 통신을 위한 서버 소켓 생성
	
	//2. 서버 소켓 주소 설정
	memset(&s_addr, 0, sizeof(s_addr)); //s_addr의 값을 모두 0으로  초기화
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP 주소 설정
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);

	//3. 서버 소켓바인딩
	if(bind(s_socket,(struct sockaddr *) &s_addr, sizeof(s_addr)) == -1){ 
		//바인딩 작업 실패 시, Cannot Bind 메시지 출력 후 프로그램 종료
		printf("Cannot Bind\n");
		return -1;
	}
	
	//4.listen() 함수 실행
	if(listen(s_socket, 5) == -1){
		printf("listen Fail\n");
		return -1;
	}

	//5. 클라이언트 요청 처리
	// 요청을 허용한 후, Hello World 메세지를 전송함
	while(1){ //무한 루프
		len = sizeof(c_addr);
		printf("클라이언트 접속을 기다리는 중....\n");
		c_socket = accept(s_socket, (struct sockaddr *)&c_addr, &len); 
		//클라이언트의 요청이 오면 허용(accept)해 주고, 해당 클라이언트와 통신할 수 있도록 클라이언트 소켓(c_socket)을 반환함.
		printf("/client is connected\n");
		printf("클라이언트 접속 허용\n");
		
		//클라이언트 접속시 메세지 출력
		write(c_socket, buffer,sizeof(buffer) );

		while(1){
			n = read(c_socket, rcvBuffer, sizeof(rcvBuffer));
			printf("rcvBuffer = %s\n",rcvBuffer);
		//클라이언트가 quit또는 kill server 입력시 클라이언트 소켓 종료
			rcvBuffer[n-1] = '\0';//개행문자 삭제
			if(strncasecmp(rcvBuffer,"quit",4)==0 ||strncasecmp(rcvBuffer,"kill server",11)==0 )
				break;
			else if(!strncasecmp(rcvBuffer,"안녕하세요",strlen("안녕하세요"))){
				strcpy(buffer,"안녕하세요. 만나서 반가워요.");
			}
		
			else if(!strncasecmp(rcvBuffer,"이름이뭐야",strlen("이름이뭐야"))){
				strcpy(buffer,"내이름은server야");
			}
			else if(!strncasecmp(rcvBuffer,"strlen ",strlen("strlen "))){
				//문자열의 길이는 xx입니다
				sprintf(buffer,"문자열의 길이는 %d입니다.",strlen(rcvBuffer)-7);
			}
			else if(!strncasecmp(rcvBuffer,"strcmp ",strlen("strcmp "))){
				char *token;
				char *str[3];
				int i=2;
				int idx = 0;
				token = strtok(rcvBuffer," ");
				printf("1: %s\n",token);
				while(token != NULL){
						str[idx] = token;
						printf("str[%d] = %s\n",idx,str[idx++]);
						token = strtok(NULL," ");
				}
				if (idx < 3)
				strcpy(buffer,"문자열 비교를 위해서는 두 문자열이 필요합니다.");
				else if(!strcmp(str[1],str[2]))//같은 문자열이면
				sprintf(buffer, "%s와 %s는 같은 문자열입니다.",str[1],str[2]);
				else	
				sprintf(buffer, "%s와 %s는 다른 문자열입니다.",str[1],str[2]);
				}
			else if(!strncmp(rcvBuffer,"readfile ",9)){
				char *token;
				char*str[10];
				int cnt = 0;
				token = strtok(rcvBuffer," "); // token =  readfile
				while(token != NULL){
				 	str[cnt++] = token; //str[0]=readfile, str[1] = <파일명> ,...
					token = strtok(NULL," ");//token <filename>
				}
				if(cnt < 2){
					strcpy(buffer,"파일명을 입력해 주세요");
					}
					else {
					FILE * fp = fopen(str[1], "r"); 
					if(fp){ //정상적으로 파일이 오픈되면
						char tempStr[BUFSIZE];
						memset(buffer,0,BUFSIZE); //buffer 초기화
						while(fgets(tempStr,BUFSIZE,(FILE *)fp)){
							strcat(buffer, tempStr); 
						//여러줄의 내용을 하나의 buffer에 저장하기위해strcat()사용 
						}
						fclose(fp);
					}
					else {//해당 파일이 없는 경우
						strcpy(buffer, "해당 파일은 존재하지 않습니다.");
						}
					}	
				}
				else if(!strncasecmp(rcvBuffer,"exec ", 5)){
					char *command;
					char *token;
					token = strtok(rcvBuffer," "); //token = exec
					command = strtok(NULL, "\0");//exec 뒤에 잇는 모든 문자열을 command 에 저장
					printf("command :%s\n",command);
					int result = system(command);//커맨드가 정상 실행되면 0,그렇지 않으면 0이 아닌 에러코드 리턴 
					if(!result){//성공한 경우
						sprintf(buffer,"[%s] 명령어가 성공했습니다.",command);
					}
					else {
						sprintf(buffer,"[%s] 명령어가 실패했습니다.",command);
					}
				
				}
			
			else
				strcpy(buffer,"무슨 말인지 모르겠습니다.");
			printf("buffer = %s\n",buffer);
			write(c_socket, buffer, strlen(buffer)); //클라이언트에게 buffer의 내용을 전송함
		}
		close(c_socket);
		//kill server 입력시 서버소켓도 종료
		if(strncasecmp(rcvBuffer,"kill server",11)==0)
			break;
	}
	close(s_socket);
	return 0;	
}
