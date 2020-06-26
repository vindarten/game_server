#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

struct factory {
	int num;
	int month;
	struct factory *next;
};

struct usrInfo {
	int fd;
	char *buf;
	int bufSize;
	struct sockaddr_in *addr;
	socklen_t addrlen;
	int number;
	int status;
	int fact;
	int raw;
	int money;
	int prod;
	struct factory *building;
;

struct gameInfo {
	struct usrInfo *usrsList;
	int numUsrs;
	int listen;
	int started;
};

struct words {
	char *word;
	struct words *next;
};

enum {initFactory = 2, initRaw = 2, initProd = 2, initMoney = 10000};
enum {blocked, active, waiting};

int initSocket(int port);
void multiplexing(struct gameInfo *gi);
void initList(struct usrInfo **ui, int numUsrs);
void clearUsr(struct usrInfo *ui);
int initSet(struct gameInfo *gi, fd_set *readfds);
void takeCom(struct gameInfo *gi, struct usrInfo *ui);
void defCom(struct gameInfo *gi, struct usrInfo *ui, struct words *list);
void putNumUsrs(int fd, int current, int numUsrs);
int addBuf(struct usrInfo *ui);
void deactivate(struct gameInfo *gi, struct usrInfo *ui);
int lineFeed(struct usrInfo *ui);
struct words *crash(struct usrInfo *ui, int lenght);
char *crtWord(struct usrInfo *ui, int j, int i);
void delList(struct words *list);
void clearBuf(struct usrInfo *ui, int lenght);
void takeUsr(struct gameInfo *gi);
void denied(int fd);
void readyCheck(struct gameInfo *gi);
int curNum(struct gameInfo *gi);
void writeStr(int fd, const char *str);
void writeInt(int fd, int num);
int writeStrAll(struct gameInfo *gi, const char *str);
int writeIntAll(struct gameInfo *gi, int num);
int cmpStr(const char *a, const char *b);
void marketInfo(struct gameInfo *gi, struct usrInfo *ui);
void playerInfo(struct gameInfo *gi, struct usrInfo *ui, struct words *list);
void delFact(struct building *first);

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
	(*gi).started = 0;
	for(;;) {
		fdMax = initSet(gi, &readfds);
		if (select(fdMax+1, &readfds, NULL, NULL, NULL) < 1) {
			continue;
		}
		for(i = 0; i < (*gi).numUsrs; i++) {
			if (((*gi).usrsList[i]).fd != -1) {
				if (FD_ISSET(((*gi).usrsList[i]).fd, &readfds)) {
					takeCom(gi, &((*gi).usrsList[i]));
				}
			}
		}
		if (FD_ISSET((*gi).listen, &readfds)) {
			takeUsr(gi);
			readyCheck(gi);
		}
	}
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
		((*ui)[i]).number = i+1;
		((*ui)[i]).status = blocked;
		((*ui)[i]).fact = initFactory;			
		((*ui)[i]).raw = initRaw;
		((*ui)[i]).money = initMoney;
		((*ui)[i]).prod = initProd;
		((*ui)[i]).building = NULL;
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
	(*ui).status = blocked;
	(*ui).factory = initFactory;
	(*ui).raw = initRaw;
	(*ui).money = initMoney;
	(*ui).prod = initProd;
	if ((*ui).building != NULL) {
		delFact((*ui).building);
	}
	(*ui).building = NULL;
}

void delFact(struct building *first)
{
	struct building *help;
	while(first != NULL) {
		help = first;
		first = (*first).next;
		free(help);
	}
}

void takeCom(struct gameInfo *gi, struct usrInfo *ui)
{
	struct words *list;
	int res;
	res = addBuf(ui);
	if (res == 0) {
		deactivate(gi, ui);
		return;
	}
	while((res = lineFeed(ui)) != -1) {
		list = crash(ui, res);
		clearBuf(ui, res);
		if (!(*gi).started) {
			writeStr((*ui).fd, "The game hasn't started yet");
			putNumUsrs((*ui).fd, curNum(gi), (*gi).numUsrs);
		} else {
			if (list != NULL) {
				defCom(gi, ui, list);
			}
		}
		delList(list);
	}
}

void takeUsr(struct gameInfo *gi)
{
	char buf[1024];
	struct sockaddr_in *addr;
	socklen_t addrlen;
	int fd, i;
	addrlen = sizeof(*addr);
	addr = malloc(addrlen);
	if ((fd = accept((*gi).listen, (struct sockaddr*)addr, &addrlen)) == -1) {
		free(addr);	
	} else if ((*gi).started) {
		denied(fd);
		free(addr);
	} else {
		for(i = 0; i < (*gi).numUsrs; i++) {
			if (((*gi).usrsList[i]).fd == -1) {
				sprintf(buf, "\nPlayer %d added", ((*gi).usrsList[i]).number);
				writeStrAll(gi, buf);
				((*gi).usrsList[i]).fd = fd;
				((*gi).usrsList[i]).addr = addr;
				((*gi).usrsList[i]).addrlen = addrlen;
				putNumUsrs(((*gi).usrsList[i]).fd, curNum(gi), (*gi).numUsrs);
				break;
			}
		}
	}
}

void readyCheck(struct gameInfo *gi)
{
	int i;
	for(i = 0; i < (*gi).numUsrs; i++) {
		if (((*gi).usrsList[i]).fd == -1) {
			return;
		}
	}
	(*gi).started = 1;
	for(i = 0; i < (*gi).numUsrs; i++) {
		((*gi).usrsList[i]).status = active;
		writeStr(((*gi).usrsList[i]).fd, "\nThe game has begun\n  >  ");
	}
}

void deactivate(struct gameInfo *gi, struct usrInfo *ui)
{
	char buf[1024];
	int i, n = 0;
	shutdown((*ui).fd, 2);
	close((*ui).fd);
	clearUsr(ui);
	sprintf(buf, "\nPlayer %d left the game", (*ui).number);
	for(i = 0; i < (*gi).numUsrs; i++) {
		if (((*gi).usrsList[i]).fd != -1) {
			writeStr(((*gi).usrsList[i]).fd, buf);
			if ((*gi).started) {
				writeStr(((*gi).usrsList[i]).fd, "\n  >  ");
			}
			n++;
		} 
	}
	if (n == 0) {
		(*gi).started = 0;
		(*gi).counter = 0;
	}
}

void delList(struct words *list)
{
	struct words *help;
	while(list != NULL) {
		free((*list).word);
		help = list;
		list = (*list).next;
		free(help);
	}
}

void defCom(struct gameInfo *gi, struct usrInfo *ui, struct words *list)
{
	if (cmpStr((*list).word, "market")) {
		marketInfo(gi, ui);
	} else if (cmpStr((*list).word, "player")) {
		playerInfo(gi, ui, (*list).next);			
	} else if (cmpStr((*list).word, "build")) {
		build(gi, ui, (*list).next);
	} else if (cmpStr((*list).word, "quit")) {
		deactivate(gi, ui);
	} else {
		writeStr((*ui).fd, "Unknown command\n");
	}
}

void build(struct gameInfo *gi, struct usrInfo *ui, struct words *list)
{
}

void marketInfo(struct gameInfo *gi, struct usrInfo *ui)
{
}

void playerInfo(struct gameInfo *gi, struct usrInfo *ui, struct words *list)
{
}

void putNumUsrs(int fd, int current, int numUsrs)
{
	char buf[1024];
	sprintf(buf, "\nThe number of connected players: %d", current);
	writeStr(fd, buf);
	sprintf(buf, "\nThe number of expected players: %d", numUsrs - current);
	writeStr(fd, buf);
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
	struct words **last;
	int i, j = 0;
	last = &list;
	for(i = 0; i < lenght; i++) {
		if ((*ui).buf[i] == ' ' || (*ui).buf[i] == '\r') {
			if (j != i) {
				help = malloc(sizeof(*help));
				(*help).word = crtWord(ui, j, i);
				(*help).next = NULL;
				*last = help;
				last = &(**last).next;
				j = i;
			}
			j++;
		}
	}
	return list;
}

char *crtWord(struct usrInfo *ui, int j, int i)
{
	int m;
	char *help;
	help = malloc((i-j+1)*sizeof(*help));
	for(m = 0; m < i-j; m++) {
		help[m] = (*ui).buf[m+j];
	}
	help[i-j] = 0;
	return help;
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

void denied(int fd)
{
	writeStr(fd, "The game has already started\n");
	shutdown(fd, 2);
	close(fd);
}

int curNum(struct gameInfo *gi)
{
	int i, n = 0;
	for(i = 0; i < (*gi).numUsrs; i++) {
		if (((*gi).usrsList[i]).fd != -1) {
			n++;
		}
	}
	return n;
}

void writeStr(int fd, const char *str)
{
	write(fd, str, strlen(str)+1);
}

void writeInt(int fd, int num)
{
	char buf[1024];
	sprintf(buf, "%d", num);
	write(fd, buf, strlen(buf)+1);
}

int writeStrAll(struct gameInfo *gi, const char *str)
{
	int i, n = 0;
	for(i = 0; i < (*gi).numUsrs; i++) {
		if (((*gi).usrsList[i]).fd != -1) {
			write(((*gi).usrsList[i]).fd, str, strlen(str)+1); 	
		}
	}
	return n;
}

int writeIntAll(struct gameInfo *gi, int num)
{
	char buf[1024];
	int i, n = 0;
	for(i = 0; i < (*gi).numUsrs; i++) {
		if (((*gi).usrsList[i]).fd != -1) {
			sprintf(buf, "%d", num);
			write(((*gi).usrsList[i]).fd, buf, strlen(buf)+1);
		}
	}
	return n;
}

int cmpStr(const char *a, const char *b)
{
	int i = 0;
	while(a[i] == b[i] && a[i] != 0) {
		i++;
	}
	return (a[i] == b[i]);
}
