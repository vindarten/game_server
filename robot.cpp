#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

const int MaxSize = 4096;
const int NameSize = 20;
const int OptSize = 20;
const int BuyRaw = 2;
const int IPAddress = 1;
const int PortNum = 2;
const int RobotName = 3;
const int Action = 4;
const int NumPlayersOrNumGame = 5;
#ifdef MYSERVER
	const int NumArg = 3;
#else
	const int NumArg = 6;
#endif

class Socket {
	int fd, BufSize;
	char buf[MaxSize+1];
	int SearchEndLine()
	{
		for(int i = 0; i < BufSize; i++)
			if (buf[i] == '\n')
				return i;
		return -1;
	}
public:
	Socket()
	{
		fd = BufSize = 0;
		buf[0] = 0;
	}
	int CheckStart()
	{
		return strcmp(buf, "  >  ");
	}
	void SendMes(const char *mes)
	{
		write(fd, mes, strlen(mes));
		printf("%s", mes);
	}
	void init(int argc, char **argv)
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
	}
	void GetStr(char *HelpBuf)
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
		#ifdef DEBUGGING
			printf("//%s\n", HelpBuf);
		#endif 
	}  
	void GetOpt(char *CmpOpt, char *HelpBuf)
	{
		char option[OptSize] = "";
		do {
			GetStr(HelpBuf);
			if (HelpBuf[0] == '&')
				sscanf(HelpBuf, "& %20s", option);
		} while(strcmp(option, CmpOpt));
	}
};

class PlayerInfo {
	int number, status, money, fact, raw, prod, AutoFact;
	char Name[NameSize];
public:
	PlayerInfo()
	{
		number = status = money = fact = raw = prod = AutoFact = 0;
		Name[0] = 0;
	}
	int CheckName(char *MyName)
	{
		return !strcmp(MyName, Name);
	}
	int GetFact()
	{
		return fact;
	}
	int GetProd()
	{
		return prod;
	}
	#ifdef MYSERVER
		void SetInfo(Socket &sock, int i);
	#else
		void SetInfo(char *HelpBuf, int i);
	#endif
	void PrintInfo();
};

#ifdef MYSERVER
	void PlayerInfo::SetInfo(Socket &sock, int i)
	{
		char HelpBuf[MaxSize];
		char string[10];
		number = i;
		sprintf(HelpBuf, "player %d\n", i); 
		sock.SendMes(HelpBuf);
		do {
			sock.GetStr(HelpBuf);
		} while(HelpBuf[0] != '#');
		sscanf(HelpBuf, "#%9s%d%d%d%d", string, &money, &fact, &raw, &prod);
		if (!strcmp(string, "active")) {
			status = 1;
		} else {
			status = 2;
		}
	}
	void PlayerInfo::PrintInfo()
	{
		printf("--------------------------------------------------\n");
		printf("    status     money   factory       raw  products\n");
		printf("%10d%10d%10d%10d%10d\n", status, money, fact, raw, prod);
		printf("--------------------------------------------------\n");
	}
#else
	void PlayerInfo::SetInfo(char *HelpBuf, int i)
	{
		number = i;
		int &r = raw, &p = prod, &m = money, &f = fact, &a = AutoFact;
		sscanf(HelpBuf,"& INFO %20s%d%d%d%d%d", Name, &r, &p, &m, &f, &a);
	}
	void PlayerInfo::PrintInfo()
	{
		int &r = raw, &p = prod, &m = money, &f = fact, &a = AutoFact;
		printf("-------------------------------------------------------\n");
		printf("                Name\n");
		printf("%20s\n", Name);
		printf("     money       raw  products   factory    AutoFactory\n");
		printf("%10d%10d%10d%10d%15d\n", m, r, p, f, a);
		printf("-------------------------------------------------------\n");
	}
#endif

class GameInfo {
	int MyNum, MaxNum, ActNum, month, level, watch;
	int raw, RawPrice, prod, ProdPrice;
	char *MyName;
public:
	GameInfo()
	{
		MyNum = MaxNum = ActNum = month = level = watch = 0;
		raw = RawPrice = prod = ProdPrice = 0;
		MyName = NULL;
	}
	int GetNum()
	{
		return MaxNum;
	}
	void StartGame(Socket &sock, char **argv);
	void SetInfo(Socket &sock);	
	void PrintInfo();
	void SetPlayerInfo(Socket &sock, PlayerInfo *pInfo);
	void apply(Socket &sock, PlayerInfo *pInfo);
	void WaitAuction(Socket &sock);
};

#ifdef MYSERVER
	void GameInfo::StartGame(Socket &sock, char **argv)
	{
		char HelpBuf[MaxSize];
		int connect, expect;
		sock.GetStr(HelpBuf);
		sock.GetStr(HelpBuf);
		sscanf(HelpBuf, "#%d%d%d", &MyNum, &connect, &expect);
		MaxNum = connect + expect;
		while(sock.CheckStart()) {
			sock.GetStr(HelpBuf);
		}
	}
	void GameInfo::SetInfo(Socket &sock)
	{
		char HelpBuf[MaxSize];
		sock.SendMes("market\n");
		do {
			sock.GetStr(HelpBuf);
		} while(HelpBuf[0] != '#');
		sscanf(HelpBuf, "#%d%d%d", &month, &level, &ActNum);
		do {
			sock.GetStr(HelpBuf);
		} while(HelpBuf[0] != '#');
		sscanf(HelpBuf, "#%d%d%d%d", &raw, &RawPrice, &prod, &ProdPrice);
	}
	void GameInfo::PrintInfo()
	{
		printf("--------------------------------------------------------\n");
		printf("      My Number   Number of Players       Active Players\n");
		printf("%15d%20d%21d\n", MyNum, MaxNum, ActNum);
		printf("--------------------------------------------------------\n");
		printf("  Current month        Market level \n");
		printf("%15d%20d\n", month, level);
		printf("--------------------------------------------------------\n");
		printf(" Bank sells: items min.price  Bank buys: items max.price\n");
		printf("%18d%10d%18d%10d\n", raw, RawPrice, prod, ProdPrice);
		printf("--------------------------------------------------------\n");
	}
	void GameInfo::SetPlayerInfo(Socket &sock, PlayerInfo *pInfo)
	{	
		int i;
		for(i = 0; i < MaxNum; i++) {
			(pInfo[i]).SetInfo(sock, i+1);
			(pInfo[i]).PrintInfo();
		} 
	}
	void GameInfo::apply(Socket &sock, PlayerInfo *pInfo)
	{
		char HelpBuf[MaxSize];
		sprintf(HelpBuf, "prod %d\n",(pInfo[MyNum-1]).GetFact());
		sock.SendMes(HelpBuf);
		(pInfo[MyNum - 1]).SetInfo(sock, MyNum);
		(pInfo[MyNum - 1]).PrintInfo();
		sprintf(HelpBuf, "sell %d %d\n",(pInfo[MyNum-1]).GetProd(),ProdPrice);
		sock.SendMes(HelpBuf);
		sprintf(HelpBuf, "buy %d %d\n", BuyRaw, RawPrice);
		sock.SendMes(HelpBuf);
		sock.SendMes("turn\n");
	}
	void GameInfo::WaitAuction(Socket &sock)
	{
		char HelpBuf[MaxSize];
		do {
			sock.GetStr(HelpBuf);
			if (HelpBuf[0] == '#') {
				printf("%s\n", HelpBuf);
			}
		} while(HelpBuf[0] != 'N');
	}
#else
	void GameInfo::StartGame(Socket &sock, char **argv)
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
		sock.SendMes("info\n");
		sock.GetOpt("PLAYERS", HelpBuf);
		sscanf(HelpBuf, "& PLAYERS %d WATCHERS %d", &MaxNum, &watch);
		ActNum = MaxNum;
		MyName = argv[RobotName];
	}
	void GameInfo::SetInfo(Socket &sock)
	{
		char HelpBuf[MaxSize];
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
	void GameInfo::SetPlayerInfo(Socket &sock, PlayerInfo *pInfo)
	{
		char HelpBuf[MaxSize], option[OptSize] = "";
		int i = 0;
		sock.SendMes("info\n");
		do {
			sock.GetStr(HelpBuf);
			if (HelpBuf[0] == '&') {
				sscanf(HelpBuf, "& %20s", option);
				if (!strcmp(option, "INFO")) {
					pInfo[i].SetInfo(HelpBuf, i);
					(pInfo[i]).PrintInfo();
					i++;
				}
			}
		} while(strcmp(option, "PLAYERS"));
		ActNum = i;
	}
	void GameInfo::apply(Socket &sock, PlayerInfo *pInfo)
	{
		char HelpBuf[MaxSize];
		for(int i = 0; i < ActNum; i++) {
			if ((pInfo[i]).CheckName(MyName)) {
				MyNum = i + 1;
				break;
			}
		}
		sprintf(HelpBuf, "prod %d\n",(pInfo[MyNum-1]).GetFact());
		sock.SendMes(HelpBuf);
		sprintf(HelpBuf, "sell %d %d\n",(pInfo[MyNum-1]).GetProd(),ProdPrice);
		sock.SendMes(HelpBuf);
		sprintf(HelpBuf, "buy %d %d\n",BuyRaw,RawPrice);
		sock.SendMes(HelpBuf);
		sock.SendMes("turn\n");
	}
	void GameInfo::WaitAuction(Socket &sock)
	{
		char HelpBuf[MaxSize], name[NameSize], option[OptSize] = "";
		int n, p;
		do {
			sock.GetStr(HelpBuf);
			if (HelpBuf[0] == '&') {
				sscanf(HelpBuf, "& %20s", option);
				if (!strcmp(option, "BOUGHT")) {
					sscanf(HelpBuf, "& BOUGHT %20s%d%d", name, &n, &p);
					printf("%s bought %d raw by price %d\n", name, n, p);
				}
				if (!strcmp(option, "SOLD")) {
					sscanf(HelpBuf, "& SOLD %20s%d%d", name, &n, &p);
					printf("%s sold %d products by price %d\n", name, n, p);
				}
			}
		} while(strcmp(option, "ENDTURN"));
	}
#endif

int main(int argc, char **argv)
{
	Socket sock;
	GameInfo gInfo;
	PlayerInfo *pInfo;
	try 
	{
		sock.init(argc, argv);
		gInfo.StartGame(sock, argv);
		pInfo = new PlayerInfo [gInfo.GetNum()];
		for(;;) {
			gInfo.SetInfo(sock);
			gInfo.PrintInfo();
			gInfo.SetPlayerInfo(sock, pInfo);
			gInfo.apply(sock, pInfo);
			gInfo.WaitAuction(sock);
		}
	}
	catch(const char *s) 
	{
		printf("%s\n", s);
	}
	delete [] pInfo;
	return 0;
}
