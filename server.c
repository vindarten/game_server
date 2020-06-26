#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "server.h"

#define FORGOTFACT "You forgot to specify the number of factories\n  >  "
#define FORGOTPROD "You forgot to specify the number of products\n  >  "
#define FORGOTPRODPRICE "You forgot to specify the price of products\n  >  "
#define FORGOTPLAYER "You forgot to specify the player number\n  >  "
#define FORGOTRAW "You forgot to specify the number of raws\n  >  "
#define FORGOTRAWPRICE "You forgot to specify the price of raw\n  >  "
#define INCORFACT "Incorrect number of factories\n  >  "
#define INCORPROD "Incorrect number of products\n  >  "
#define INCORRAW "Incorrect number of raws\n  >  "
#define INCORPRICE "Incorrect price\n  >  "
#define INCORNUM "Incorrect user number\n  > "
#define CANT "You can't use this command now\n  >  "
#define NOTENOUGHMONEY "You don't have enough money\n  >  "
#define NOTENOUGHRAW "You don't have enough raw\n  >  "
#define NOTENOUGHFACT "You don't have enough factories\n  >  "
#define NOTENOUGHPROD "You don't have enough products\n  >  "
#define HIGHPRICE "The price is too high\n  >  "
#define LOWPRICE "The price is too low\n  >  "
#define ACCEPT "The application is accepted\n  >  "
#define LEFTGAME "The player already left the game\n  >  "
#define BANKBOUGHT "\nBank buy %d products from player %d for price %d"
#define BANKSOLD "\nBank sold %d raws to player %d for price %d"
#define PLAYERBUY "\nPlayer %d want to buy %d raws for price %d"
#define PLAYERSELL "\nPlayer %d want to sell %d products for price %d"
#define MARKET "market                - get information about the market\n"
#define PLAYER "player <number>       - get information about the player\n"
#define PROD "prod <number>         - produce <number> items of product\n"
#define BUY "buy <number> <price>  - participate in the auction of raw\n"
#define SELL "sell <number> <price> - participate in the auction of products\n"
#define BUILD "build <number>        - build <number> new factories\n"
#define HELP "help                  - get help\n"
#define WHO "whoAmI                - check your number\n"
#define TURN "turn                  - finish action this turn\n"
#define QUIT "quit                  - leave the game\n  >  "

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
	(gi).rawApp = (gi).prodApp = NULL;
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
	(*gi).month = 0;
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

void initMarket(struct gameInfo *gi)
{
	(*gi).month = 1;
	(*gi).level = startLevel;
	itemsAndPrice(gi);
	delItems(&(*gi).rawApp);	
	delItems(&(*gi).prodApp);
}

void itemsAndPrice(struct gameInfo *gi)
{
	int n;
	n = curNum(gi);
	(*gi).raw = (int)((0.5 + 0.5 * (*gi).level) * n);
	(*gi).prod = (int)((3.5 - 0.5 * (*gi).level) * n);
	(*gi).prodPrice = 7000 - 500 * (*gi).level;
	if ((*gi).level == 1) {
		(*gi).rawPrice = 800;
	} else if ((*gi).level == 2) {
		(*gi).rawPrice = 650;
	} else if ((*gi).level == 3) {
		(*gi).rawPrice = 500;
	} else if ((*gi).level == 4) {
		(*gi).rawPrice = 400;
	} else {
		(*gi).rawPrice = 300;
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
		((*ui)[i]).number = i+1;
		((*ui)[i]).status = blocked;
		((*ui)[i]).fact = ((*ui)[i]).availFact = initFactory;
		((*ui)[i]).raw = initRaw;
		((*ui)[i]).prod = ((*ui)[i]).availProd = initProd;
		((*ui)[i]).money = initMoney;
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
	(*ui).fact = (*ui).availFact = initFactory;
	(*ui).raw = initRaw;
	(*ui).prod = (*ui).availProd = initProd;
	(*ui).money = initMoney;
	delFact(&(*ui).building);
}

void delFact(struct factory **first)
{
	struct factory *help;
	while(*first != NULL) {
		help = *first;
		*first = (**first).next;
		free(help);
	}
}

void delItems(struct items **first)
{
	struct items *help;
	while(*first != NULL) {
		help = *first;
		*first = (**first).next;
		free(help);
	}
}

void takeCom(struct gameInfo *gi, struct usrInfo *ui)
{
	struct words *list;
	int res;
	res = addBuf(ui);
	if (res == 0 || (*ui).bufSize > maxBufSize ) {
		deactivate(gi, ui, leave);
		return;
	}
	while((res = lineFeed(ui)) != -1) {
		list = crash(ui, res);
		clearBuf(ui, res);
		if (!(*gi).month) {
			writeStr((*ui).fd, "The game hasn't started yet");
			putNumUsrs((*ui).fd, curNum(gi), (*gi).numUsrs);
		} else {
			if (list != NULL) {
				defCom(gi, ui, list);
			} else {
				writeStr((*ui).fd, "  >  ");
			}
		}
		delList(list);
	}
	checkAtStep(gi);
}

void takeUsr(struct gameInfo *gi)
{
	char buf[128];
	struct sockaddr_in *addr;
	socklen_t addrlen;
	int fd, i;
	addrlen = sizeof(*addr);
	addr = malloc(addrlen);
	if ((fd = accept((*gi).listen, (struct sockaddr*)addr, &addrlen)) == -1) {
		free(addr);	
	} else if ((*gi).month) {
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
				sprintf(buf, "\nYou are Player %d", ((*gi).usrsList[i]).number);
				writeStr(((*gi).usrsList[i]).fd, buf);
				putNumUsrs(((*gi).usrsList[i]).fd, curNum(gi), (*gi).numUsrs);
				break;
			}
		}
	}
	if ((*gi).month == 0) {
		checkAtBegin(gi);
	}
}

void checkAtStep(struct gameInfo *gi)
{
	int i;
	for(i = 0; i < (*gi).numUsrs; i++) {
		if (((*gi).usrsList[i]).fd != -1) {
			if (((*gi).usrsList[i]).status != waiting) {
				return;
			}
		}
	}
	turnEnd(gi);
}

void checkAtBegin(struct gameInfo *gi)
{
	int i, fd;
	for(i = 0; i < (*gi).numUsrs; i++) {
		if (((*gi).usrsList[i]).fd == -1) {
			return;
		}
	}
	initMarket(gi);
	for(i = 0; i < (*gi).numUsrs; i++) {
		((*gi).usrsList[i]).status = active;
		fd = ((*gi).usrsList[i]).fd;
		writeStr(fd, "\nThe game has begun. Type 'help' to get help\n  >  ");
	}
}

void turnEnd(struct gameInfo *gi)
{
	char buf[128];
	(*gi).month++;
	writeStrAll(gi, "\nThe auction began");
	whoWant(gi, (*gi).rawApp, (*gi).prodApp);
	sort(&(*gi).rawApp, dec);
	sort(&(*gi).prodApp, inc);
	auctionRaw(gi);
	auctionProd(gi);
	removeCosts((*gi).usrsList, (*gi).numUsrs);
	buildFactory((*gi).usrsList, (*gi).numUsrs, (*gi).month);
	changeLevel(gi);
	itemsAndPrice(gi);
	checkBankrupt(gi, (*gi).usrsList);
	sprintf(buf, "\nNew market level: %d\n  >  ", (*gi).level);
	writeStrAll(gi, buf); 
	activate((*gi).usrsList, (*gi).numUsrs);
}

void whoWant(struct gameInfo *gi, struct items *r, struct items *p)
{
	char buf[128];
	while(r != NULL) {
		sprintf(buf, PLAYERBUY, (*r).numPlayer, (*r).num, (*r).price); 
		writeStrAll(gi, buf);
		r = (*r).next;
	}
	while(p != NULL) {
		sprintf(buf, PLAYERSELL, (*p).numPlayer, (*p).num, (*p).price); 
		writeStrAll(gi, buf);
		p = (*p).next;
	}
}

void auctionRaw(struct gameInfo *gi)
{
	int raw, i, j, n;
	struct items *help;
	while((*gi).rawApp != NULL && (*gi).raw > 0) {
		if ((raw = numItems((*gi).rawApp, &n)) <= (*gi).raw) {
			(*gi).raw -= raw;
			for(i = 1; i <= n; i++) {
				help = (*gi).rawApp;
				(*gi).rawApp = (*(*gi).rawApp).next;
				j = (*help).numPlayer - 1;
				((*gi).usrsList[j]).raw += (*help).num;
				((*gi).usrsList[j]).money -= (*help).num * (*help).price;
				printBankSold(gi, j + 1, (*help).num, (*help).price);
				free(help);
			}
		} else {
			chooseRawApp(gi, n);
		}
	}
	delItems(&(*gi).rawApp);
}

void chooseRawApp(struct gameInfo *gi, int n)
{
	int r, i;
	struct items **cur;
	struct items *help;
	r = 1 + (int)(n * rand() / (RAND_MAX + 1.0));
	cur = &(*gi).rawApp;
	for(i = 1; i < r; i++) {
		cur = &(**cur).next;
	}
	help = *cur;
	*cur = (**cur).next;
	i = (*help).numPlayer - 1;
	if ((*help).num <= (*gi).raw) {
		(*gi).raw -= (*help).num;
		((*gi).usrsList[i]).raw += (*help).num;
		((*gi).usrsList[i]).money -= (*help).num * (*help).price;
		printBankSold(gi, i + 1, (*help).num, (*help).price);
	} else {
		((*gi).usrsList[i]).raw += (*gi).raw;
		((*gi).usrsList[i]).money -= (*gi).raw * (*help).price;
		printBankSold(gi, i + 1, (*gi).raw, (*help).price);
		(*gi).raw = 0;
	}
	free(help);
}

void auctionProd(struct gameInfo *gi)
{
	int prod, i, j, n;
	struct items *help;
	while((*gi).prodApp != NULL && (*gi).prod > 0) {
		if ((prod = numItems((*gi).prodApp, &n)) <= (*gi).prod) {
			(*gi).prod -= prod;
			for(i = 1; i <= n; i++) {
				help = (*gi).prodApp;
				(*gi).prodApp = (*(*gi).prodApp).next;
				j = (*help).numPlayer - 1;
				((*gi).usrsList[j]).prod -= (*help).num;
				((*gi).usrsList[j]).money += (*help).num * (*help).price;
				printBankBuy(gi, j + 1, (*help).num, (*help).price);
				free(help);
			}
		} else {
			chooseProdApp(gi, n);
		}
	}
	delItems(&(*gi).prodApp);
}

void chooseProdApp(struct gameInfo *gi, int n)
{
	int r, i;
	struct items **cur;
	struct items *help;
	r = 1 + (int)(n * rand() / (RAND_MAX + 1.0));
	cur = &(*gi).prodApp;
	for(i = 1; i < r; i++) {
		cur = &(**cur).next;
	}
	help = *cur;
	*cur = (**cur).next;
	i = (*help).numPlayer - 1;
	if ((*help).num <= (*gi).prod) {
		(*gi).prod -= (*help).num;
		((*gi).usrsList[i]).prod -= (*help).num;
		((*gi).usrsList[i]).money += (*help).num * (*help).price;
		printBankBuy(gi, i + 1, (*help).num, (*help).price);
	} else {
		((*gi).usrsList[i]).prod -= (*gi).prod;
		((*gi).usrsList[i]).money += (*gi).prod * (*help).price;
		printBankBuy(gi, i + 1, (*gi).prod, (*help).price);
		(*gi).prod = 0;
	}
	free(help);
}

void printBankSold(struct gameInfo *gi, int i, int n, int p)
{
	char buf[128];
	sprintf(buf, BANKSOLD, n, i, p);
	writeStrAll(gi, buf);
}

void printBankBuy(struct gameInfo *gi, int i, int n, int p)
{
	char buf[128];
	sprintf(buf, BANKBOUGHT, n, i, p);
	writeStrAll(gi, buf);
}

int numItems(struct items *first, int *n)
{
	int item, max;
	*n = 1;
	max = (*first).price;
	item = (*first).num;
	first = (*first).next;
	while(first != NULL && (*first).price == max) {
		item += (*first).num;
		first = (*first).next;
		(*n)++;
	}
	return item;
}

void checkBankrupt(struct gameInfo *gi, struct usrInfo *ui)
{
	int i;
	for(i = 0; i < (*gi).numUsrs; i++) {
		if ((ui[i]).fd != -1) {
			if ((ui[i]).money < 0) {
				writeStr((ui[i]).fd, "\nYou are a bankrupt\n");
				deactivate(gi, &(ui[i]), bankrupt);
			}
		}
	}
}

void sort(struct items **first, int order)
{
	struct items **cur;
	struct items **minAddr;
	struct items *help;
	struct items *new = NULL;
	int min;
	while(*first != NULL) {
		min = (**first).price;
		minAddr = first;
		cur = &(**first).next;
		while(*cur != NULL) {
			if (order == dec) {
				if ((**cur).price < min) {
					min = (**cur).price;
					minAddr = cur;
				} 
			} else {
				if ((**cur).price > min) {
					min = (**cur).price;
					minAddr = cur;
				}
			}
			cur = &(**cur).next;
		}
		help = *minAddr;
		*minAddr = (**minAddr).next;
		(*help).next = new;
		new = help;
	}
	*first =new;
}

void removeCosts(struct usrInfo *ui, int numUsrs)
{
	int i;
	for(i = 0; i < numUsrs; i++) {
		if ((ui[i]).fd != -1) {
			(ui[i]).money -= (ui[i]).fact * monCostFact;
			(ui[i]).money -= (ui[i]).prod * monCostProd;
			(ui[i]).money -= (ui[i]).raw * monCostRaw;
		}
	}
}

void buildFactory(struct usrInfo *ui, int numUsrs, int month)
{
	struct factory **cur;
	struct factory *help;
	int i;
	for(i = 0; i < numUsrs; i++) {
		if ((ui[i]).fd != -1) {
			cur = &(ui[i]).building;
			while(*cur != NULL) {
				if ((**cur).month == month) {
					(ui[i]).fact += (**cur).num;
					(ui[i]).money -= (**cur).num * costOfFact;
					help = *cur;
					*cur = (**cur).next;
					free(help);
				} else {
					cur = &(**cur).next;
				}
			}
		}
	}
}

void activate(struct usrInfo *ui, int numUsrs)
{
	int i;
	for(i = 0; i < numUsrs; i++) {
		if ((ui[i]).fd != -1) {
			(ui[i]).availFact = (ui[i]).fact;
			(ui[i]).availProd = (ui[i]).prod;
			(ui[i]).status = active;
		}
	}
}

void changeLevel(struct gameInfo *gi)
{
	const int probArr[5][5] = {
		{ 4, 4, 2, 1, 1 },
		{ 3, 4, 3, 1, 1 },
		{ 1, 3, 4, 3, 1 },
		{ 1, 1, 3, 4, 3 },
		{ 1, 1, 2, 4, 4 }
	};
	int r, j, sum = 0;
	r = 1 + (int)(12.0 * rand() / (RAND_MAX + 1.0));
	for(j = 0; j < 5 ; j++) {
		sum += probArr[(*gi).level - 1][j];
		if (sum >= r) {
			(*gi).level = j + 1;
			break;
		}
	}
}

void deactivate(struct gameInfo *gi, struct usrInfo *ui, int stat)
{
	char buf[1024];
	int i, j, n = 0;
	shutdown((*ui).fd, 2);
	close((*ui).fd);
	clearUsr(ui);
	if (stat == leave) {
		sprintf(buf, "\nPlayer %d left the game", (*ui).number);
	} else {
		sprintf(buf, "\nPlayer %d is bankrupt", (*ui).number);
	}
	for(i = 0; i < (*gi).numUsrs; i++) {
		if (((*gi).usrsList[i]).fd != -1) {
			j = i;
			writeStr(((*gi).usrsList[i]).fd, buf);
			if ((*gi).month && stat == leave) {
				writeStr(((*gi).usrsList[i]).fd, "\n  >  ");
			}
			n++;
		} 
	}
	if (n == 0) {
		(*gi).month = 0;
	} else if (n == 1) {
		writeStr(((*gi).usrsList[j]).fd, "\nYou won\n");
		deactivate(gi, &((*gi).usrsList[j]), leave);
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
		market(gi, (*ui).fd);
	} else if (cmpStr((*list).word, "player")) {
		player(gi, (*ui).fd, (*list).next);
	} else if (cmpStr((*list).word, "prod")) {
		prod(gi, ui, (*list).next);
	} else if (cmpStr((*list).word, "build")) {
		build(gi, ui, (*list).next);
	} else if (cmpStr((*list).word, "sell")) {
		sell(gi, ui, (*list).next);
	} else if (cmpStr((*list).word, "buy")) {
		buy(gi, ui, (*list).next);
	} else if (cmpStr((*list).word, "help")) {
		help((*ui).fd);
	} else if (cmpStr((*list).word, "whoAmI")) {
		whoAmI(ui);
	} else if (cmpStr((*list).word, "turn")) {
		endStep(gi, ui);
	} else if (cmpStr((*list).word, "quit")) {
		deactivate(gi, ui, leave);
	} else {
		writeStr((*ui).fd, "Unknown command. Type 'help' to get help\n  >  ");
	}
}

void market(struct gameInfo *gi, int fd)
{
	char buf[128];
	sprintf(buf, "Current month: %d\n", (*gi).month);
	write(fd, buf, strlen(buf)+1);
	sprintf(buf, "Current market level: %d\n", (*gi).level);
	write(fd, buf, strlen(buf)+1);
	writeStr(fd, "Players still active:\n");
	sprintf(buf, "#%19d\n", curNum(gi));
	write(fd, buf, strlen(buf)+1);  
	writeStr(fd, "Bank sells:    items min.price\n");
	sprintf(buf, "#%19d%10d\n", (*gi).raw, (*gi).rawPrice);
	write(fd, buf, strlen(buf)+1);  
	writeStr(fd, "Bank buys:     items max.price\n");
	sprintf(buf, "#%19d%10d\n  >  ", (*gi).prod, (*gi).prodPrice);
	write(fd, buf, strlen(buf)+1);  
}

void player(struct gameInfo *gi, int fd, struct words *list)
{
	char *endptr = NULL;
	int i, n;
	if (list == NULL) {
		writeStr(fd, FORGOTPLAYER);
	} else {
		i = strtol((*list).word, &endptr, 10);
		n = (*gi).numUsrs;
		if ((*list).word[0] != '\0' && *endptr == '\0' && i > 0 && i <= n) {
			if (((*gi).usrsList[i-1]).fd != -1) {
				printPlayer((*gi).usrsList[i-1], fd);
			} else {
				writeStr(fd, LEFTGAME);
			}
		} else {
			writeStr(fd, INCORNUM);
		}
	}
}

void printPlayer(struct usrInfo ui, int fd)
{
	char buf[128];
	writeStr(fd, "     money   factory       raw  products\n");
	sprintf(buf, "#%9d%10d%10d%10d\n", ui.money, ui.fact, ui.raw, ui.prod);
	write(fd, buf, strlen(buf)+1);
	writeStr(fd, "  >  ");
}

void prod(struct gameInfo *gi, struct usrInfo *ui, struct words *list)
{
	char *endptr = NULL;
	int numProd;
	if ((*ui).status == waiting) {
		writeStr((*ui).fd, CANT);
	} else if (list == NULL) {
		writeStr((*ui).fd, FORGOTPROD);
	} else {
		numProd = strtol((*list).word, &endptr, 10);
		if ((*list).word[0] != '\0' && *endptr == '\0') {
			if (numProd * costOfProd > (*ui).money) {
				writeStr((*ui).fd, NOTENOUGHMONEY);
			} else if (numProd > (*ui).raw) {
				writeStr((*ui).fd, NOTENOUGHRAW);
			} else if (numProd > (*ui).availFact) {
				writeStr((*ui).fd, NOTENOUGHFACT);
			} else {
				(*ui).money -= numProd * costOfProd;
				(*ui).raw -= numProd;
				(*ui).availFact -=numProd;
				(*ui).prod += numProd;
				(*ui).availProd += numProd;
				writeStr((*ui).fd, ACCEPT);
			}
		} else {
			writeStr((*ui).fd, INCORPROD);
		}
	}
}

void build(struct gameInfo *gi, struct usrInfo *ui, struct words *list)
{
	char *endptr = NULL;
	int numFact;
	if ((*ui).status == waiting) {
		writeStr((*ui).fd, CANT);
	} else if (list == NULL) {
		writeStr((*ui).fd, FORGOTFACT);
	} else {
		numFact = strtol((*list).word, &endptr, 10);
		if ((*list).word[0] != '\0' && *endptr == '\0') {
			if (numFact * costOfFact > (*ui).money) {
				writeStr((*ui).fd, NOTENOUGHMONEY);
			} else {
				(*ui).money -= numFact * costOfFact;
				addFact(&((*ui).building), numFact, (*gi).month);
				writeStr((*ui).fd, ACCEPT);
			}
		} else {
			writeStr((*ui).fd, INCORFACT);
		}
	}
}

void sell(struct gameInfo *gi, struct usrInfo *ui, struct words *list)
{
	char *endptr1, *endptr2;
	int numProd, price;
	if ((*ui).status == waiting) {
		writeStr((*ui).fd, CANT);
	} else if (list == NULL) {
		writeStr((*ui).fd, FORGOTPROD);
	} else if ((*list).next == NULL) {
		writeStr((*ui).fd, FORGOTPRODPRICE);
	} else {
		numProd = strtol((*list).word, &endptr1, 10);
		price = strtol((*(*list).next).word, &endptr2, 10);
		if ((*(*list).next).word[0] == '\0' || *endptr2 != '\0')  {
			writeStr((*ui).fd, INCORPRICE);
		} else if ((*list).word[0] == '\0' || *endptr1 != '\0') {
			writeStr((*ui).fd, INCORPROD);
		} else {
			if (numProd > (*ui).availProd) {
				writeStr((*ui).fd, NOTENOUGHPROD);
			} else if (price > (*gi).prodPrice) {
				writeStr((*ui).fd, HIGHPRICE);
			} else {
				(*ui).availProd -= numProd;
				addItems(&((*gi).prodApp), numProd, price, (*ui).number);
				writeStr((*ui).fd, ACCEPT);
			}
		}
	}
}

void buy(struct gameInfo *gi, struct usrInfo *ui, struct words *list)
{
	char *endptr1, *endptr2;
	int numRaw, price;
	if ((*ui).status == waiting) {
		writeStr((*ui).fd, CANT);
	} else if (list == NULL) {
		writeStr((*ui).fd, FORGOTRAW);
	} else if ((*list).next == NULL) {
		writeStr((*ui).fd, FORGOTRAWPRICE);
	} else {
		numRaw = strtol((*list).word, &endptr1, 10);
		price = strtol((*(*list).next).word, &endptr2, 10);
		if ((*(*list).next).word[0] == '\0' || *endptr2 != '\0')  {
			writeStr((*ui).fd, INCORPRICE);
		} else if ((*list).word[0] == '\0' || *endptr1 != '\0') {
			writeStr((*ui).fd, INCORRAW);
		} else {
			if (price < (*gi).rawPrice) {
				writeStr((*ui).fd, LOWPRICE);
			} else {
				addItems(&((*gi).rawApp), numRaw, price, (*ui).number);
				writeStr((*ui).fd, ACCEPT);
			}
		}
	}
}

void addItems(struct items **first, int num, int price, int i)
{
	struct items *help;
	help = malloc(sizeof(*help));
	(*help).num = num;
	(*help).price = price;
	(*help).numPlayer = i;
	(*help).next = *first;
	*first = help;
}

void help(int fd)
{
	writeStr(fd, MARKET PLAYER PROD BUY SELL BUILD HELP WHO TURN QUIT);
}

void whoAmI(struct usrInfo *ui)
{
	char buf[128];	
	sprintf(buf, "You are Player %d\n  >  ", (*ui).number);
	writeStr((*ui).fd, buf);
}

void endStep(struct gameInfo *gi, struct usrInfo *ui)
{
	if ((*ui).status == active) {
		(*ui).status = waiting;
		writeStr((*ui).fd, "  >  ");
	} else {
		writeStr((*ui).fd, CANT);
	}
}

void addFact(struct factory **first, int numFact, int month)
{
	struct factory *help;
	help = malloc(sizeof(*help));
	(*help).month = month + constTime;
	(*help).num = numFact;
	(*help).next = *first;
	*first = help;
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
