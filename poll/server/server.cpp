#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
 #include <unistd.h>
 #include <string.h>

 #include <arpa/inet.h>

 #include <poll.h>
#include <vector>





typedef std::vector<struct pollfd> PollFdList;

int main(int argc, char * argv[])
{

	//1. create socket
	int socketfd  = 0;

	if(socketfd = socket(AF_INET, SOCK_STREAM, 0) < 0)
	{
		fprintf(stderr, "create socket failed!");
		return -1;
	}



	//2. bind
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));

	//set value
	servaddr.sin_family= AF_INET;
	servaddr.sin_port = htons(5188);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


	if(bind(socketfd, (struct sockaddr *) &servaddr, sizeof(struct sockaddr)) < 0)
	{
		perror("failed to bind");
		close(socketfd);
		return -1;
	}
	//3. listen
	if(listen(socketfd, SOMAXCONN) < 0)
	{
		perror("failed to listen");
		close(socketfd);
		return -1;
	}

	//4. poll
	struct pollfd pfd;

	pfd.fd = socketfd;
	pfd.events= POLLIN;

	PollFdList pollfds;

	pollfds.push_back(pfd);

	int nready = 0;


	int connfd = 0;
	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	while(true)
	{
		nready = poll(pollfds.data(), pollfds.size(), -1);

		if(nready ==  -1)
		{
			if(EINTR == errno)
			{
				continue;
			}
			perror("poll failed");
			return -1;
		}


		if(nready == 0)
		{
			continue;
		}

		
		//first element is socketfd

		if(pollfds[0].revents & POOLIN)
		{
			peerlen = sizeof(peeraddr);
			connfd = accept(socketfd, (struct sockaddr*)&peeraddr, &peerlen);
			
			if(connfd == -1)
			{
				perror("failed to accept");
				close(socketfd);
				return -1;
			}


			pfd.fd = connfd;
			pfd.events= POLLIN;
			pfd.revents = 0;
			pollfds.push_back(pfd);
			nready --;


			printf("has a connect come")

		}




	}

	

	close(socketfd);

	return 0;
}
