#include "GameInfo.h"

int Socket::SearchEndLine() const
{
	for(int i = 0; i < BufSize; i++)
		if (buf[i] == '\n')
			return i;
	return -1;
}

void Socket::CheckEnd(char *HelpBuf) const
{
	char option[OptSize] = "";
	if (HelpBuf[0] == '&') {
		sscanf(HelpBuf, "& %20s", option);
		if (!strcmp(option, "YOU_WIN"))
			throw "You won!\n";
	}
}

Socket::Socket(int argc, char **argv)
{
	struct sockaddr_in addr;
	if (argc < NumArg)
		throw "Not enough arguments";	
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
		throw "socket returned -1";
	addr.sin_family = AF_INET;
	addr.sin_port = htons(strtol(argv[PortNum], NULL, 10));
	if (!inet_aton(argv[IPAddress], &(addr.sin_addr)))
		throw "inet_aton returned 0";
	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
		throw "connect returned -1";
	BufSize = 0;
	buf[0] = 0;
}

void Socket::SendMes(const char *mes) const
{
	write(fd, mes, strlen(mes));
	printf("%s", mes);
}

void Socket::GetStr(char *HelpBuf)
{
	char AddBuf[MaxSize];
	int AddSize, i, n;
	while((n = SearchEndLine()) == -1) {
		AddSize = read(fd, AddBuf, MaxSize);
		if (BufSize + AddSize > MaxSize)
			throw "Buffer is full";
		if (AddSize == 0)
			throw "The connection with server is lost";
		for(i = 0; i < AddSize; i++)
			buf[BufSize + i] = AddBuf[i];
		BufSize += AddSize;
		buf[BufSize] = 0;
	}
	for(i = 0; i < n; i++)
		HelpBuf[i] = buf[i];
	HelpBuf[n] = 0;
	for(i = 0; i < BufSize - n; i++)
		buf[i] = buf[i + n + 1];
	BufSize -= n + 1;
	CheckEnd(HelpBuf);
#ifdef DEBUGGING
	printf("//%s\n", HelpBuf);
#endif 
}
 
void Socket::GetOpt(const char *CmpOpt, char *HelpBuf)
{
	char option[OptSize] = "";
	do {
		GetStr(HelpBuf);
		if (HelpBuf[0] == '&')
			sscanf(HelpBuf, "& %20s", option);
	} while(strcmp(option, CmpOpt));
}

int PlayerInfo::GetDif(AuctItem *item) const
{
	if (item == 0)
		return 0;
	int res = 0, i = 0;
	while(item != 0) {
		printf("!%d %d\n", item->MaxPrice, item->price);
		res += item->MaxPrice - item->price;
		if (item->MaxPrice - item->price != 0)
			i++;
		item = item->next;
	}
	if (i) {
		return res / i;
	} else {
		return 0;
	}
}

int PlayerInfo::GetLastDif(AuctItem *item, int month) const
{
	if (item == 0) {
		return 0;
	} else {
		if (item->month == month - 1) {
			return item->MaxPrice - item->price;
		} else {
			return 0;
		}
	}
}

int PlayerInfo::GetLast(AuctItem *item, int month) const
{
	if (item == 0) {
		return 0;
	} else {
		if (item->month == month - 1) {
			return item->price;
		} else {
			return 0;
		}
	}
}

void PlayerInfo::Delete(AuctItem *item)
{
	AuctItem *help;
	while(item != 0) {
		help = item;
		item = item->next;
		delete help;
	}
}

PlayerInfo::PlayerInfo()
{
	number = active = money = fact = raw = prod = AutoFact = 0;
	name[0] = 0;
	bought = sold = 0;
}

void PlayerInfo::InitInfo(char *HelpBuf, int i)
{
	number = i;
	active = 1;
	int &r = raw, &p = prod, &m = money, &f = fact, &a = AutoFact;
	sscanf(HelpBuf,"& INFO %20s%d%d%d%d%d", name, &r, &p, &m, &f, &a);
}

void PlayerInfo::SetInfo(char *HelpBuf)
{
	active = 1;
	int &r = raw, &p = prod, &m = money, &f = fact, &a = AutoFact;
	sscanf(HelpBuf,"& INFO %20s%d%d%d%d%d", name, &r, &p, &m, &f, &a);
}

void PlayerInfo::AuctResBought(char *HelpBuf, int month, int MaxPrice)
{
	int number, price;
	sscanf(HelpBuf, "& BOUGHT %20s%d%d", name, &number, &price);
	bought = new AuctItem(number, price, month, MaxPrice, bought);
}

void PlayerInfo::AuctResSold(char *HelpBuf, int month, int MaxPrice)
{
	int number, price;
	sscanf(HelpBuf, "& SOLD %20s%d%d", name, &number, &price);
	sold = new AuctItem(number, price, month, MaxPrice, sold);
}

void PlayerInfo::PrintInfo()
{
	int &r = raw, &p = prod, &m = money, &f = fact, &a = AutoFact;
	printf("-------------------------------------------------------\n");
	printf("                                         Name    Status\n");
	if (active)
		printf("%45s    active\n", name);
	else
		printf("%45s  deactive\n", name);
	printf("     money       raw  products   factory    AutoFactory\n");
	printf("%10d%10d%10d%10d%15d\n", m, r, p, f, a);
	printf("-------------------------------------------------------\n");
}

void GameInfo::EnterGame(char **argv)
{
	char HelpBuf[MaxSize];
	int i = 0, n = strtol(argv[NumPlayersOrNumGame], NULL, 10) ;
	sprintf(HelpBuf, "%s\n", argv[RobotName]);
	sock.SendMes(HelpBuf);
	if (strcmp(argv[Action], "create")) {
		sprintf(HelpBuf, ".join %d\n", n);
		sock.SendMes(HelpBuf);
		sock.GetOpt("START", HelpBuf);
	} else {
		sock.SendMes(".create\n");
		while(i < n) {
			sock.GetStr(HelpBuf);
			if (HelpBuf[0] == '@' && HelpBuf[1] == '+') {
				printf("New player added\n");
				i++;
			}
			if (HelpBuf[0] == '@' && HelpBuf[1] == '-') {
				printf("PLayer left the game\n");
				i--;
			}
		}
		sock.SendMes("start\n");
	}
}

GameInfo::GameInfo(int argc, char **argv)
	:sock(argc, argv), month(0)
{
	char HelpBuf[MaxSize];
	EnterGame(argv);
	sock.SendMes("info\n");
	sock.GetOpt("PLAYERS", HelpBuf);
	sscanf(HelpBuf, "& PLAYERS %d", &MaxNum);
	pInfo = new PlayerInfo[MaxNum];
	MyName = argv[RobotName];
	sock.SendMes("info\n");
	for(int i = 0; i < MaxNum; i++) {
		sock.GetOpt("INFO", HelpBuf);
		pInfo[i].InitInfo(HelpBuf, i);
		if (pInfo[i].CheckName(MyName))
			MyNum = i;
	}
	sock.SendMes("market\n");
	sock.GetOpt("MARKET", HelpBuf);
	sscanf(HelpBuf,"& MARKET %d%d%d%d",&raw,&RawPrice,&prod,&ProdPrice);
}

void GameInfo::UpdateInfo()
{
	char HelpBuf[MaxSize], option[OptSize] = "", name[NameSize] = "";
	int i;
	for(i = 0; i < MaxNum; i++)
		pInfo[i].Deactive();
	sock.SendMes("info\n");
	do {
		sock.GetStr(HelpBuf);
		if (HelpBuf[0] == '&') {
			sscanf(HelpBuf, "& %20s %20s", option, name);
			if (!strcmp(option, "INFO")) {
				for(i = 0; i < MaxNum; i++) {
					if (!strcmp(name, pInfo[i].GetName())) {
						pInfo[i].SetInfo(HelpBuf);
						break;
					}
				}
			}
		}
	} while(strcmp(option, "PLAYERS"));
	sock.SendMes("market\n");
	sock.GetOpt("MARKET", HelpBuf);
	sscanf(HelpBuf,"& MARKET %d%d%d%d",&raw,&RawPrice,&prod,&ProdPrice);
}

void GameInfo::PrintInfo()
{
	printf("--------------------------------------------------------\n");
	printf(" Bank sells: items min.price  Bank buys: items max.price\n");
	printf("%18d%10d%18d%10d\n", raw, RawPrice, prod, ProdPrice);
	printf("--------------------------------------------------------\n");
}

void GameInfo::WaitAuction()
{
	char HelpBuf[MaxSize], name[NameSize], option[OptSize] = "";
	int i;
	sock.SendMes("turn\n");
	do {
		sock.GetStr(HelpBuf);
		if (HelpBuf[0] == '&') {
			sscanf(HelpBuf, "& %20s %20s", option, name);
			if (!strcmp(option, "BOUGHT")) {
				for(i = 0; i < MaxNum; i++) {
					if (!strcmp(name, pInfo[i].GetName())) {
						pInfo[i].AuctResBought(HelpBuf, month, RawPrice);
						break;
					}
				}	
			}
			if (!strcmp(option, "SOLD")) {
				for(i = 0; i < MaxNum; i++) {
					if (!strcmp(name, pInfo[i].GetName())) {
						pInfo[i].AuctResSold(HelpBuf, month, ProdPrice);
						break;
					}
				}	
			}
			if (!strcmp(option, "BANKRUPT")) {
				sscanf(HelpBuf, "& BANKRUPT %20s", name);
				if (!strcmp(name, MyName)) 
					throw "You are bankrupt";
			}
		}
	} while(strcmp(option, "ENDTURN"));
	month++;
}

void GameInfo::SendProd(long long num) const
{
	char HelpBuf[MaxSize];
	sprintf(HelpBuf, "prod %lld\n", num);
	sock.SendMes(HelpBuf);
}

void GameInfo::SendSell(long long prod, long long price) const
{
	char HelpBuf[MaxSize];
	sprintf(HelpBuf, "sell %lld %lld\n", prod, price);
	sock.SendMes(HelpBuf);
}

void GameInfo::SendBuy(long long raw, long long price) const
{
	char HelpBuf[MaxSize];
	sprintf(HelpBuf, "buy %lld %lld\n", raw, price);
	sock.SendMes(HelpBuf);
}

void GameInfo::SendBuild() const
{
	char HelpBuf[MaxSize];
	sprintf(HelpBuf, "build\n");
	sock.SendMes(HelpBuf);
}

void GameInfo::SendAbuild() const
{
	char HelpBuf[MaxSize];
	sprintf(HelpBuf, "abuild\n");
	sock.SendMes(HelpBuf);
}

void GameInfo::SendUpgrade() const
{
	char HelpBuf[MaxSize];
	sprintf(HelpBuf, "upgrade\n");
	sock.SendMes(HelpBuf);
}
