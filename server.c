#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

struct usrInfo {
	int fd;
	char *buf;
	int bufSize;
	struct sockaddr_in *addr;
	socklen_t addrlen;
};

struct gameInfo {
	struct usrInfo *usrsList;
	int numUsrs;
	int listen;
	int counter;
};

struct words {
	char *word;
	struct words *next;
};

int initSocket(int port);
void multiplexing(struct gameInfo *gi);
void initList(struct usrInfo **ui, int numUsrs);
void clearUsr(struct usrInfo *ui);
int initSet(struct gameInfo *gi, fd_set *readfds);
void takeCom(struct gameInfo *gi, struct usrInfo *usrsList, int i);
int addBuf(struct usrInfo *ui);
void unplug(struct usrInfo *ui);
int lineFeed(struct usrInfo ui);
struct words *crash(struct usrInfo *ui, int lenght);
void clearBuf(struct usrInfo *ui, int lenght);
void takeUsr(struct gameInfo *gi, struct usrInfo *usrsList);
void writeStr(int fd, const char *str);
int cmpStr(const char *a, const char *b);

int main(int argc, char **argv)
{
	struct gameInfo gi;	
	int port;
	if (argc < 3) {
		printf("error\n");
		return 0;
	}	
	gi.numUsrs = strtol(argv[1], NULL, 10);
	port = strtol(argv[2], NULL, 10);
	gi.counter = 0;
	if ((gi.listen = initSocket(port)) == -1) {
		printf("error\n");
		return 0;
	}
	multiplexing(&gi);
	return 0;
}

int initSocket(int port)
{
	struct sockaddr_in addr;
	int opt = 1, fd;
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return -1;
	}
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		return -1;
	}
	if (listen(fd, 5) == -1) {
		return -1;
	}
	return fd;
}

void multiplexing(struct gameInfo *gi)
{
	int i, fdMax;
	fd_set readfds;
	initList(&((*gi).usrsList), (*gi).numUsrs);
	for(;;) {
		fdMax = initSet(gi, &readfds);
		if (select(fdMax+1, &readfds, NULL, NULL, NULL) < 1) {
			continue;
		}
		for(i = 0; i < (*gi).numUsrs; i++) {
			if ((usrsList[i]).fd != -1) {
				if (FD_ISSET((usrsList[i]).fd, &readfds)) {
					takeCom(gi, &((*gi).usrsList[i]));
				}
			}
		}
		if (FD_ISSET((*gi).listen, &readfds)) {
			takeUsr(gi, usrsList);
		}
	}
}

void initList(struct usrInfo **ui, int numUsrs)
{
	int i;
	*ui = malloc(numUsrs*sizeof(**ui));
	for(i = 0; i < numUsrs; i++) {
		((*ui)[i]).fd = -1;
		((*ui)[i]).buf = NULL;
		((*ui)[i]).bufSize = 0;
		((*ui)[i]).addr = NULL;
		((*ui)[i]).addrlen = 0;
	}
}

void clearUsr(struct usrInfo *ui)
{
	(*ui).fd = -1;
	if ((*ui).buf != NULL) {
		free((*ui).buf);
	}
	(*ui).buf = NULL;
	(*ui).bufSize = 0;
	if ((*ui).addr != NULL) {
		free((*ui).addr);
	}
	(*ui).addr = NULL;
	(*ui).addrlen = 0;
}

int initSet(struct gameInfo *gi, fd_set *readfds)
{
	int i, fdMax;
	FD_ZERO(readfds);
	FD_SET((*gi).listen, readfds);
	fdMax = (*gi).listen;
	for(i = 0; i < (*gi).numUsrs; i++) {
		if (((*gi).usrsList[i]).fd != -1) {
			FD_SET(((*gi).usrsList[i]).fd, readfds);
			if (((*gi).usrsList[i]).fd > fdMax) {
				fdMax = ((*gi).usrsList[i]).fd;
			}
		}
	}
	return fdMax;
}

void takeCom(struct gameInfo *gi, struct usrInfo *usrsList)
{
	struct words *list;
	char buf[1024];
	int res;
	res = addBuf(usrsList);
	if (res == 0) {
		unplug(usrsList);
		return;
	}
	while((res = lineFeed(usrsList)) != -1) {
		list = crash(&(usrsList[i]), res);
		clearBuf(&(usrsList[i]), res);
		if (list != NULL) {
			if (cmpStr((*list).word, "up")) {
				(*gi).counter++;		
			}
			if (cmpStr((*list).word, "down")) {
				(*gi).counter--;		
			}
			if (cmpStr((*list).word, "show")) {
				sprintf(buf, "%d\n", (*gi).counter);
				write((usrsList[i]).fd, buf, strlen(buf)+1);
			}
			if (cmpStr((*list).word, "quit")) {
				unplug(&(usrsList[i]));
			}
		}
	}
	writeStr((usrsList[i]).fd, "  >  ");
}

void unplug(struct usrInfo *ui)
{
	shutdown((*ui).fd, 2);
	close((*ui).fd);
	clearUsr(ui);
}

int addBuf(struct usrInfo *ui)
{
	char buf[1024];
	int j, res;
	char *new;
	res = read((*ui).fd, buf, sizeof(buf));
	if (res > 0) {
		new = malloc(sizeof(*new)*((*ui).bufSize + res));
		for(j = 0; j < (*ui).bufSize; j++) {
			new[j] = (*ui).buf[j];
		}
		for(j = 0; j < res; j++) {
			new[j+(*ui).bufSize] = buf[j];
		}
		if ((*ui).buf != NULL) {
			free((*ui).buf);
		}
		(*ui).buf = new;
		(*ui).bufSize += res;
	}
	return res;
}

int lineFeed(struct usrInfo *ui)
{
	int i;
	for(i = 0; i < (*ui).bufSize; i++) {
		if ((*ui).buf[i] == '\n') {
			return i;
		}
	}
	return -1;
}

struct words *crash(struct usrInfo *ui, int lenght)
{
	struct words *list = NULL;
	struct words *help;
	int m, i, j = 0;
	for(i = 0; i < lenght; i++) {
		if ((*ui).buf[i] == ' ' || (*ui).buf[i] == '\r') {
			if (j != i) {
				help = malloc(sizeof(*help));
				(*help).word = malloc(sizeof(*(*help).word)*(i-j+1));
				for(m = 0; m < i-j; m++) {
					(*help).word[m] = (*ui).buf[m+j];
				}
				(*help).word[i-j] = 0;
				(*help).next = list;
				list = help;
				j = i;
			}
			j++;
		}
	}
	return list;
}

void clearBuf(struct usrInfo *ui, int lenght)
{
	char *new = NULL;
	int i, n;
	n = (*ui).bufSize - (lenght + 1);
	if (n != 0) {
		new = malloc(n*sizeof(*new));
		for(i = 0; i < n; i++) {
			new[i] = (*ui).buf[lenght + 1 +	i];
		}
	}
	free((*ui).buf);
	(*ui).buf = new;
	(*ui).bufSize = n;
}

void takeUsr(struct gameInfo *gi, struct usrInfo *usrsList)
{
	struct sockaddr_in *addr;
	socklen_t addrlen;
	int fd, i;
	addrlen = sizeof(*addr);
	addr = malloc(addrlen);
	if ((fd = accept((*gi).listen, (struct sockaddr*)addr, &addrlen)) == -1) {
		free(addr);	
		return;
	}
	for(i = 0; i < (*gi).numUsrs; i++) {
		if ((usrsList[i]).fd == -1) {
			(usrsList[i]).fd = fd;
			(usrsList[i]).addr = addr;
			(usrsList[i]).addrlen = addrlen;
			writeStr((usrsList[i]).fd, "  >  ");
			break;
		}
		if (i == (*gi).numUsrs-1) {
			writeStr(fd, "The game has already started\n");
			shutdown(fd, 2);
			close(fd);
			free(addr);
		}
	}
}

void writeStr(int fd, const char *str)
{
	write(fd, str, strlen(str)+1);
}

int cmpStr(const char *a, const char *b)
{
	int i = 0;
	while(a[i] == b[i] && a[i] != 0) {
		i++;
	}
	return (a[i] == b[i]);
}
