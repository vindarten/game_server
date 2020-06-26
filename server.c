#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

struct usrInfo {
	int fd;
};

struct gameInfo {
	int numUsrs;
	int listen;
};

int initSocket(int port);
int multiplexing(struct gameInfo *gi);
int initSet(struct gameInfo *gi, struct usrInfo *usrsList, fd_set *readfds);
int interact(struct gameInfo *gi, struct usrInfo *usrsList, fd_set *readfds);

int main(int argc, char **argv)
{
	struct gameInfo gi;	
	int port;
	if (argc < 3) {
		printf("error\n");
		return 0;
	}	
	gi.numUsrs = atoi(argv[1]);
	port = atoi(argv[2]);
	if ((gi.listen = initSocket(port)) == -1) {
		printf("error\n");
		return 0;
	}
	if (multiplexing(&gi))
		printf("error\n");
	return 0;
}

int initSocket(int port)
{
	struct sockaddr_in addr;
	int fd;
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
		return -1;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) 
		return -1;
	if (listen(fd, 5) == -1)
		return -1;
	return fd;
}

int multiplexing(struct gameInfo *gi)
{
	struct usrInfo *usrsList;
	int i, fdMax;
	fd_set readfds;
	usrsList = malloc((*gi).numUsrs*sizeof(*usrsList));
	for(i = 0; i < (*gi).numUsrs; i++)
		(usrsList[i]).fd = -1;
	for(;;) {
		fdMax = initSet(gi, usrsList, &readfds);
		i = select(fdMax+1, &readfds, NULL, NULL, NULL);
		if (i < 1) {
			free(usrsList);
			return -1;
		}
		if (interact(gi, usrsList, &readfds)) {
			free(usrsList);
			return -1;
		}
	}
}

int initSet(struct gameInfo *gi, struct usrInfo *usrsList, fd_set *readfds)
{
	int i, fdMax;
	FD_ZERO(readfds);
	FD_SET((*gi).listen, readfds);
	fdMax = (*gi).listen;
	for(i = 0; i < (*gi).numUsrs; i++)
		if ((usrsList[i]).fd != -1) {
			FD_SET((usrsList[i]).fd, readfds);
			if ((usrsList[i]).fd > fdMax)
				fdMax = (usrsList[i]).fd;
		}
	return fdMax;
}

int interact(struct gameInfo *gi, struct usrInfo *usrsList, fd_set *readfds)
{
	int i, fdHelp;
	for(i = 0; i < (*gi).numUsrs; i++)
		if ((usrsList[i]).fd != -1)
			if (FD_ISSET((usrsList[i]).fd, readfds)) 
			{/*read*/}
	if (FD_ISSET((*gi).listen, readfds)) {
		if ((fdHelp = accept((*gi).listen, NULL, NULL)) == -1)	
			return -1;
		for(i = 0; i < (*gi).numUsrs; i++) {
			if ((usrsList[i]).fd == -1) {
				(usrsList[i]).fd = fdHelp;
				break;
			}
			if (i == (*gi).numUsrs-1) 
			{/*denied*/}
		}
	}
	return 0;
}
