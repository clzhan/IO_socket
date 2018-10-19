#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
 #include <unistd.h>
 #include <string.h>

 #include <arpa/inet.h>

 #include <poll.h>
#include <vector>
#include <errno.h>





typedef std::vector<struct pollfd> PollFdList;

int main(int argc, char * argv[])
{

	//1. create socket
	int socketfd  = 0;

	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd <= 0)
	{
		fprintf(stderr, "create socket failed!");
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		return -1;
	}

	printf("socketfd = %d\n", socketfd);

	//2. bind
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));

	//set value
	servaddr.sin_family= AF_INET;
	servaddr.sin_port = htons(6000);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


	if(bind(socketfd, (struct sockaddr *) &servaddr, sizeof(struct sockaddr)) < 0)
	{
		perror("failed to bind");
		printf("bind error: %s(errno: %d)\n", strerror(errno), errno);
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

		printf("nready %d\n", nready);		
		//first element is socketfd

		if(pollfds[0].revents & POLLIN)
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


			printf("has a connect coming remote ip %s port %d connfd %d\n",inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port),connfd);

			if(nready == 0)
			{
				continue;
			}

		}

		for(PollFdList::iterator itor = pollfds.begin() + 1 ; 
				itor != pollfds.end() && nready > 0 ; 
				++itor)
		{
			if(itor->revents & POLLIN)
			{

				printf("read a buff .....\n");
				--nready;
				connfd = (*itor).fd;

				char buf[1024] = {0};

				int ret = read(connfd, buf, 1024);

				if(-1 == ret)
				{
					perror("recv error");
					return -1;
				}

				if(0 == ret)
				{
					printf("Client close ...");
					itor = pollfds.erase(itor);
					--itor;

					close(connfd);
					continue;
				}

				printf("%s\n", buf);
				write(connfd, buf, strlen(buf));

			}
		}
	}

	

	close(socketfd);

	return 0;
}
