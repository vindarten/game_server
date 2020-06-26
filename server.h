#ifndef SERVER_HANDLER
	#define SERVER_HANDLER
	struct factory {
		int num, month;
		struct factory *next;
	};

	struct items {
		int num, numPlayer, price;
		struct items *next;
	};

	struct usrInfo {
		int fd, bufSize, number, status; 
		int fact, availFact, raw, prod, availProd, money;
		char *buf;
		struct factory *building;
	};

	struct gameInfo {
		struct usrInfo *usrsList;
		struct items *rawApp;
		struct items *prodApp;
		int numUsrs, listen, month, level, raw, rawPrice, prod, prodPrice;
	};

	struct words {
		char *word;
		struct words *next;
	};

	enum {initFactory = 2, initRaw = 4, initProd = 2, initMoney = 10000};
	enum {costOfProd = 2000, costOfFact = 2500, constTime = 5};
	enum {monCostProd = 500, monCostFact = 1000, monCostRaw = 300};
	enum {blocked, active, waiting};
	enum {inc, dec};
	enum {startLevel = 3, maxBufSize = 1024};
	enum {leave, bankrupt};

	int initSocket(int port);
	void multiplexing(struct gameInfo *gi);
	void itemsAndPrice(struct gameInfo *gi);
	void clearUsr(struct usrInfo *ui);
	int initSet(struct gameInfo *gi, fd_set *readfds);
	void takeCom(struct gameInfo *gi, struct usrInfo *ui);
	void defCom(struct gameInfo *gi, struct usrInfo *ui, struct words *list);
	void putNumUsrs(int fd, int number, int current, int numUsrs);
	int addBuf(struct usrInfo *ui);
	void whoWant(struct gameInfo *gi, struct items *r, struct items *p);
	void sort(struct items **first, int order);
	void auctionRaw(struct gameInfo *gi);
	void auctionProd(struct gameInfo *gi);
	void chooseRawApp(struct gameInfo *gi, int n);
	void chooseProdApp(struct gameInfo *gi, int n);
	void delItems(struct items **first);
	void printBankSold(struct gameInfo *gi, int i, int n, int p);
	void printBankBuy(struct gameInfo *gi, int i, int n, int p);
	int numItems(struct items *first, int *n);
	void removeCosts(struct usrInfo *ui, int numUsrs);
	void checkBankrupt(struct gameInfo *gi, struct usrInfo *ui);
	void buildFactory(struct usrInfo *ui, int numUsrs, int month);
	void activate(struct usrInfo *ui, int numUsrs);
	void changeLevel(struct gameInfo *gi);
	void deactivate(struct gameInfo *gi, struct usrInfo *ui, int stat);
	int lineFeed(struct usrInfo *ui);
	struct words *crash(struct usrInfo *ui, int lenght);
	char *crtWord(struct usrInfo *ui, int j, int i);
	void delList(struct words *list);
	void clearBuf(struct usrInfo *ui, int lenght);
	void takeUsr(struct gameInfo *gi);
	void checkAtBegin(struct gameInfo *gi);
	void turnEnd(struct gameInfo *gi);
	int curNum(struct gameInfo *gi);
	void writeStr(int fd, const char *str);
	int writeStrAll(struct gameInfo *gi, const char *str);
	int cmpStr(const char *a, const char *b);
	void market(struct gameInfo *gi, int fd);
	void player(struct gameInfo *gi, int fd, struct words *list);
	void printPlayer(struct usrInfo ui, int fd, const char *st);
	void delFact(struct factory **first);
	void build(struct gameInfo *gi, struct usrInfo *ui, struct words *list);
	void sell(struct gameInfo *gi, struct usrInfo *ui, struct words *list);
	void buy(struct gameInfo *gi, struct usrInfo *ui, struct words *list);
	void prod(struct gameInfo *gi, struct usrInfo *ui, struct words *list);
	void whoAmI(struct usrInfo *ui);
	void endStep(struct gameInfo *gi, struct usrInfo *ui);
	void addFact(struct factory **first, int numFact, int month);
	void addItems(struct items **first, int num, int price, int i);
#endif
