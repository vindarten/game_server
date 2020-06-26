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
const int AnsSize = 20;
const int BuyRaw = 2;
const int IPAddress = 1;
const int PortNum = 2;
const int RobotName = 3;
const int Action = 4;
const int NumPlayers = 5;
const int NumGame = 5;

class Socket {
	int fd, BufSize;
	char buf[MaxSize+1];
	void add(char *AddBuf, int AddSize)
	{
		int i;
		if (BufSize + AddSize > MaxSize) {
			throw "Buffer is full";
		}
		for(i = 0; i < AddSize; i++) {
			buf[BufSize + i] = AddBuf[i];
		}
		BufSize += AddSize;
		buf[BufSize] = 0;
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
	void init(int argc, char **argv);
	void GetStr(char *HelpBuf);
	void StartGame(char **argv);
};

void Socket::init(int argc, char **argv)
{
	struct sockaddr_in addr;
	if (argc < 6) {
		throw "Not enough arguments";
	}	
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		throw "socket returned -1";
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(strtol(argv[PortNum], NULL, 10));
	if (!inet_aton(argv[IPAddress], &(addr.sin_addr))) {
		throw "inet_aton returned 0";
	}
	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		throw "connect returned -1";
	}
}
	
void Socket::GetStr(char *HelpBuf)
{
	char AddBuf[MaxSize];
	int AddSize, i, n;
	while (1) {
		n = -1;
		for(i = 0; i < BufSize; i++) {
			if (buf[i] == '\n') {
				n = i;
				break;
			}
		}
		if (n != -1 && BufSize > 0) {
			for(i = 0; i < n; i++) {
				HelpBuf[i] = buf[i];
			}
			HelpBuf[n] = 0;
			for(i = 0; i < BufSize - n; i++) {
				buf[i] = buf[i + n + 1];
			}
			BufSize -= n + 1;
			#ifdef DEBUGGING
				printf("//%s\n", HelpBuf);
			#endif
			return;
		} else {			
			AddSize = read(fd, AddBuf, MaxSize);
			if (AddSize == 0) {
				throw "The connection with server is lost";
			}
			add(AddBuf, AddSize);
		}
	}
}  
#ifndef MYSERVER
	void Socket::StartGame(char **argv)
	{
		char HelpBuf[MaxSize];
		char answer[AnsSize] = "";
		int i = 0, n;
		sprintf(HelpBuf, "%s\n", argv[RobotName]);
		SendMes(HelpBuf);
		if (strcmp(argv[Action], "create")) {
			n = strtol(argv[NumGame], NULL, 10);
			sprintf(HelpBuf, ".join %d\n", n);
			SendMes(HelpBuf);
			do {
				GetStr(HelpBuf);
				if (HelpBuf[0] == '&') {
					sscanf(HelpBuf, "& %20s", answer);
				}
			} while(strcmp(answer, "START"));
		} else {
			SendMes(".create\n");
			n = strtol(argv[NumPlayers], NULL, 10);
			while(i < n) {
				GetStr(HelpBuf);
				if (HelpBuf[0] == '@') {
					if (HelpBuf[1] == '+') {
						i++;
						printf("New player added\n");
					}
					if (HelpBuf[1] == '-') {
						i--;
						printf("PLayer left the game\n");
					}
				}
			}
			SendMes("start\n");
		}
	}
#endif

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
	#ifdef MYSERVER
		void SetNum(const char *HelpBuf);
	#else
		void SetActNum(Socket &sock);
		void SetNum(Socket &sock, char **argv);
	#endif
	void SetInfo(Socket &sock);	
	void PrintInfo();
	void apply(Socket &sock, PlayerInfo *pInfo);
	void WaitAuction(Socket &sock);
};

#ifdef MYSERVER
	void GameInfo::SetNum(const char *HelpBuf)
	{
		int connect, expect;
		sscanf(HelpBuf, "#%d%d%d", &MyNum, &connect, &expect);
		MaxNum = connect + expect;
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
	void GameInfo::apply(Socket &sock, PlayerInfo *pInfo)
	{
		char HelpBuf[MaxSize];
		sprintf(HelpBuf, "prod %d\n", (pInfo[MyNum - 1]).GetFact());
		sock.SendMes(HelpBuf);
		(pInfo[MyNum - 1]).SetInfo(sock, MyNum);
		(pInfo[MyNum - 1]).PrintInfo();
		sprintf(HelpBuf, "sell %d %d\n", (pInfo[MyNum - 1]).GetProd(), ProdPrice);
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
		printf("%s\n", HelpBuf);
	}
#else
	void GameInfo::SetActNum(Socket &sock)
	{
		char HelpBuf[MaxSize];
		do {
			sock.GetStr(HelpBuf);
		} while(HelpBuf[0] != '&');
		sscanf(HelpBuf, "& PLAYERS %d WATCHERS %d", &MaxNum, &watch);
		ActNum = MaxNum;
		sock.GetStr(HelpBuf);
	}

	void GameInfo::SetNum(Socket &sock, char **argv)
	{
		int i = 0;
		char HelpBuf[MaxSize];
		sock.SendMes("info\n");
		while (i < 2) {
			sock.GetStr(HelpBuf);
			if (HelpBuf[0] == '#') 
				i++;
		}
		SetActNum(sock);
		MyName = argv[RobotName];
	}
	void GameInfo::SetInfo(Socket &sock)
	{
		char HelpBuf[MaxSize];
		sock.SendMes("market\n");
		do {
			sock.GetStr(HelpBuf);
		} while(HelpBuf[0] != '&');
		sscanf(HelpBuf,"& MARKET %d%d%d%d",&raw,&RawPrice,&prod,&ProdPrice);
		sock.GetStr(HelpBuf);
	}
	void GameInfo::PrintInfo()
	{
		printf("--------------------------------------------------------\n");
		printf("        My Name   Number of Players       Active Players\n");
		printf("%15s%20d%21d\n", MyName, MaxNum, ActNum);
		printf("--------------------------------------------------------\n");
		printf(" Bank sells: items min.price  Bank buys: items max.price\n");
		printf("%18d%10d%18d%10d\n", raw, RawPrice, prod, ProdPrice);
		printf("--------------------------------------------------------\n");
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
		sprintf(HelpBuf, "prod %d\n", (pInfo[MyNum - 1]).GetFact());
		sock.SendMes(HelpBuf);
		sprintf(HelpBuf, "sell %d %d\n", (pInfo[MyNum - 1]).GetProd(), ProdPrice);
		sock.SendMes(HelpBuf);
		sprintf(HelpBuf, "buy %d %d\n", BuyRaw, RawPrice);
		sock.SendMes(HelpBuf);
		sock.SendMes("turn\n");
	}
	void GameInfo::WaitAuction(Socket &sock)
	{
		char HelpBuf[MaxSize];
		char answer[AnsSize] = "";
		do {
			sock.GetStr(HelpBuf);
			if (HelpBuf[0] == '&') {
				sscanf(HelpBuf, "& %20s", answer);
			}
		} while(strcmp(answer, "ENDTURN"));
	}
#endif

#ifdef MYSERVER
	void MakeTurn(Socket &sock, GameInfo &gInfo, PlayerInfo *pInfo)
	{	
		int i;
		gInfo.SetInfo(sock);
		gInfo.PrintInfo();
		for(i = 0; i < gInfo.GetNum(); i++) {
			(pInfo[i]).SetInfo(sock, i+1);
			(pInfo[i]).PrintInfo();
		} 
		gInfo.apply(sock, pInfo);
		gInfo.WaitAuction(sock);
	}
	void WaitStart(Socket &sock, GameInfo &gInfo)
	{
		char HelpBuf[MaxSize];
		sock.GetStr(HelpBuf);
		sock.GetStr(HelpBuf);
		gInfo.SetNum(HelpBuf);
		while(sock.CheckStart()) {
			sock.GetStr(HelpBuf);
		}
	}
#else
	void MakeTurn(Socket &sock, GameInfo &gInfo, PlayerInfo *pInfo)
	{
		int i = 0, j = 0;
		char HelpBuf[MaxSize];
		gInfo.SetInfo(sock);
		sock.SendMes("info\n");
		do {
			sock.GetStr(HelpBuf);
			if (HelpBuf[0] == '#') {
				j++;
			}
			if (HelpBuf[0] == '&') {
				pInfo[i].SetInfo(HelpBuf, i);
				i++;
			}
		} while(j < 2);
		gInfo.SetActNum(sock);
		gInfo.PrintInfo();
		for(i = 0; i < gInfo.GetNum(); i++) {
			(pInfo[i]).PrintInfo();
		}
		gInfo.apply(sock, pInfo);
		gInfo.WaitAuction(sock);
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
		#ifdef MYSERVER
			WaitStart(sock, gInfo);
		#else 
			sock.StartGame(argv);
			gInfo.SetNum(sock, argv);
		#endif
		pInfo = new PlayerInfo [gInfo.GetNum()];
		for(;;) {
			MakeTurn(sock, gInfo, pInfo);
		}
	}
	catch(const char *s) 
	{
		printf("%s\n", s);
	}
	delete [] pInfo;
	return 0;
}
