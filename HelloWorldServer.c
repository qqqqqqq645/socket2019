#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

#define PORT 10000
#define BUF_SIZE 100
#define ENDCLIENT "quit\n"

char buffer[100] = "server connercted\n";
//char endclient[100] = "quit";

int main(){
	int c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	char stringData[BUF_SIZE];
	int len;
	int n;
	int i;
	char transBuffer[BUF_SIZE];

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

		n = strlen(buffer);
		write(c_socket, buffer, n); //클라이언트에게 buffer의 내용을 전송함
		
		while(1){
			n = read(c_socket,stringData,sizeof(stringData)); 
			//data read

			if(n<0){//read failed
				printf("Read Failed\n");
				return -1;
			}

		
			stringData[n]='\0'; //prevent crash
			n=strlen(stringData);		
			printf("client : %s\n",stringData);
			if(strcmp(stringData,ENDCLIENT)==0) {
				printf("quit 입력으로 연결 종료\n");
				break;
			}
			printf("ff\n");
			printf("n = %d\n",n);
			strcpy(transBuffer,stringData);
			n=strlen(transBuffer);
			write(c_socket,transBuffer,n);
		}
	}
	close(s_socket);
	return 0;	
}
