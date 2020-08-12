#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

#define SERV_PORT 9999

int main(int argc, char * argv[])
{
	int lfd, cfd;
	char buf[BUFSIZ];

	struct sockaddr_in serv_addr, clit_addr;
	socklen_t clit_addr_len = sizeof(clit_addr);

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	listen(lfd, 128);

	fd_set rset, allset;
	FD_ZERO(&allset);
	FD_SET(lfd, &allset);
	int n, maxfd = lfd;

	while(1)
	{
		rset = allset;
		n = select(maxfd+1, &rset, NULL, NULL, NULL);
		if (n < 0)
		{
			perror("select error");
			exit(1);
		}
		if (FD_ISSET(lfd, &rset))
		{
			cfd = accept(lfd, (struct sockaddr*)&clit_addr, &clit_addr_len);
			FD_SET(cfd, &allset);
			if(cfd > maxfd)
				maxfd = cfd;
			if(n == 1)
				continue;
		}
		for (int i = lfd + 1; i <= maxfd; i++)
		{
			if(FD_ISSET(i, &rset))
			{
				int m = read(i, buf, sizeof(buf));
				if(m == 0)
				{
					close(i);
					FD_CLR(i, &allset);
				}
				else if (m > 0)
				{
					for(int j = 0; j < m; j++)
					{
						buf[j] = tolower(buf[j]);
					}
					write(i, buf, m);
					write(STDOUT_FILENO, buf, m);
				}
			}
		}
	}
	close(lfd);
	return 0;
}
