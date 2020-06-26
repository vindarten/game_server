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
const int BuyRaw = 2;
const int IPAddress = 1;
const int PortNum = 2;

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
	void init(int argc, char **argv);
	void printing() const
	{
		printf("@%s\n", buf);
	}
	void GetStr(char *HelpBuf);
	int CheckStart()
	{
		return strcmp(buf, "  >  ");
	}
	void SendMes(const char *mes)
	{
		write(fd, mes, strlen(mes));
	}
};

void Socket::init(int argc, char **argv)
{
	struct sockaddr_in addr;
	if (argc < 3) {
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

class PlayerInfo {
	int number, status, money, fact, raw, prod;
public:
	PlayerInfo()
	{
		number = status = money = fact = raw = prod = 0;
	}
	void SetInfo(Socket &sock, int i)
	{
		char HelpBuf[MaxSize];
		char string[10];
		sprintf(HelpBuf, "player %d\n", i); 
		sock.SendMes(HelpBuf);
		printf("player %d\n", i);
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
	void PrintInfo()
	{
		printf("--------------------------------------------------\n");
		printf("    status     money   factory       raw  products\n");
		printf("%10d%10d%10d%10d%10d\n", status, money, fact, raw, prod);
		printf("--------------------------------------------------\n");
	}
	int GetFact()
	{
		return fact;
	}
	int GetProd()
	{
		return prod;
	}
};

class GameInfo {
	int MyNum, MaxNum, ActNum, month, level, raw, RawPrice, prod, ProdPrice;
public:
	GameInfo()
	{
		MyNum = MaxNum = ActNum = month = level = 0;
		raw = RawPrice = prod = ProdPrice = 0;
	}
	void SetNum(const char *HelpBuf)
	{
		int connect, expect;
		sscanf(HelpBuf, "#%d%d%d", &MyNum, &connect, &expect);
		MaxNum = connect + expect;
	}
	int GetNum()
	{
		return MaxNum;
	}
	void SetInfo(Socket &sock)
	{
		char HelpBuf[MaxSize];
		sock.SendMes("market\n");
		printf("market\n");
		do {
			sock.GetStr(HelpBuf);
		} while(HelpBuf[0] != '#');
		sscanf(HelpBuf, "#%d%d%d", &month, &level, &ActNum);
		do {
			sock.GetStr(HelpBuf);
		} while(HelpBuf[0] != '#');
		sscanf(HelpBuf, "#%d%d%d%d", &raw, &RawPrice, &prod, &ProdPrice);
	}
	void PrintInfo()
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
	void apply(Socket &sock, PlayerInfo *pInfo)
	{
		char HelpBuf[MaxSize];
		sprintf(HelpBuf, "prod %d\n", (pInfo[MyNum - 1]).GetFact());
		sock.SendMes(HelpBuf);
		printf("%s", HelpBuf);
		(pInfo[MyNum - 1]).SetInfo(sock, MyNum);
		(pInfo[MyNum - 1]).PrintInfo();
		sprintf(HelpBuf, "sell %d %d\n", (pInfo[MyNum - 1]).GetProd(), ProdPrice);
		sock.SendMes(HelpBuf);
		printf("%s", HelpBuf);
		sprintf(HelpBuf, "buy %d %d\n", BuyRaw, RawPrice);
		sock.SendMes(HelpBuf);
		printf("%s", HelpBuf);
		sock.SendMes("turn\n");
		printf("turn\n");
	}
	void WaitAuction(Socket sock)
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
};

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

int main(int argc, char **argv)
{
	Socket sock;
	GameInfo gInfo;
	PlayerInfo *pInfo;
	try 
	{
		sock.init(argc, argv);
		#ifdef DEBUGGING
			printf("Established!\n");
		#endif
		WaitStart(sock, gInfo);
		#ifdef DEBUGGING
			printf("Start!\n");
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
