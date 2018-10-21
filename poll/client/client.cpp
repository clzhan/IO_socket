#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include <arpa/inet.h>

#include <string.h>
#include <unistd.h>





int main(int argc, char * argv[])
{
	//create socket
	int socketfd = 0;

	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd < 0)
	{
		printf("create socketfd error: %s errno %d\n", strerror(errno),errno);
		return -1;
	}

	
	//connect 
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6000);

	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(socketfd, (struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
	{
		printf("connect error: %s errno %d\n", strerror(errno),errno);
		return -1;

	}

	
	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};

	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{
		if(strncmp(sendbuf, "quit",4) == 0)
		{
			break;
		}
		write(socketfd, sendbuf, strlen(sendbuf));

		int ret = read(socketfd, recvbuf, sizeof(recvbuf));
		
		if(0 == ret)
		{
			break;
		}




		fputs(recvbuf, stdout);

		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));

	}
	
	close(socketfd);

	return 0;
}
