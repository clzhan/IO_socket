#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
 #include <unistd.h>
 #include <string.h>

 #include <arpa/inet.h>

 #include <sys/wait.h>
 #include <sys/epoll.h>
#include <vector>
#include <errno.h>
#include<string>
#include <iostream>
#include <algorithm>





typedef std::vector<struct epoll_event> EventList;

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

	//4. epoll
	
	std::vector<int> clients;

	int epollfd = epoll_create1(EPOLL_CLOEXEC);
	
	struct epoll_event event;

	event.data.fd = socketfd;
	event.events= EPOLLIN;

	epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &event);


	EventList events(16);



	int nready = 0;

	int connfd = 0;
	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	while(true)
	{
		nready = epoll_wait(epollfd,events.data(), events.size(), -1);

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

		if(nready == events.size())
		{
			events.resize(events.size() * 2);
		}

		printf("nready %d\n", nready);		
		//first element is socketfd

		for(int i = 0; i < nready; i++)
		{
			if(events[i].data.fd == socketfd)
			{
				peerlen = sizeof(peeraddr);
				connfd = accept(socketfd, (struct sockaddr*)&peeraddr, &peerlen);

				if(connfd == -1)
				{
					perror("failed to accept");
					close(socketfd);
					return -1;
				}
				

				printf("has a connect coming remote ip %s port %d connfd %d\n",inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port),connfd);

				clients.push_back(connfd);

				event.data.fd = connfd;
				event.events = EPOLLIN;
				epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);

			}
			else if(events[i].events & EPOLLIN)
			{
				connfd = events[i].data.fd;

				if(connfd < 0)
				{
					continue;
				}

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

					close(connfd);

					epoll_ctl(epollfd, EPOLL_CTL_DEL,connfd, &events[i]);

					//del in clients
				    clients.erase(std::remove(clients.begin(), clients.end(), connfd), clients.end());	

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
