#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <strings.h>

#define SERV_PORT 6666

int main(int argc, char * argv[])
{
	int i, j, n, nready;
	int maxfd = 0;
	int lfd, cfd;
	char buf[BUFSIZ];

	struct sockaddr_in serv_addr, clit_addr;
	socklen_t clit_addr_len = sizeof(clit_addr);

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(lfd, 128);

	fd_set rset, allset;
	maxfd = lfd;

	FD_ZERO(&allset);
	FD_SET(lfd, &allset);

	while(1)
	{
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(nready < 0)
		{
			perror("select error");
		}
		
		if(FD_ISSET(lfd, &rset))
		{
			cfd = accept(lfd, (struct sockaddr*)&clit_addr, &clit_addr_len);
			FD_SET(cfd, &allset);
			if(maxfd < cfd)
				maxfd = cfd;
			if(0 == --nready)
				continue; 
		}
		for(i = lfd + 1; i <= maxfd; i++)
		{
			if(FD_ISSET(i, &rset))
			{
				n = read(i, buf, sizeof(buf));
				if (n == 0)
				{
					close(i);
					FD_CLR(i, &allset);
				}
				else if (n > 0)
				{
					for(j = 0; j < n; j++)
					{
						buf[j] = toupper(buf[j]);
					}
					write(i, buf, n);
				}
			}
		}
	}
}
