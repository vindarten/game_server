#ifndef GAMEINFO_H
#define GAMEINFO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "MyString.h"

enum {MaxSize = 4096, NameSize = 20, OptSize = 20};
enum {IPAddress = 1, PortNum, Script, RobotName, Action, NumPlayersOrNumGame};
enum {NumArg = 7};

class Socket {
	int fd, BufSize;
	char buf[MaxSize+1];
	int SearchEndLine() const;
	void CheckEnd(char *HelpBuf) const;
public:
	Socket(int argc, char **argv);
	int CheckStart() const { return MyStrcmp(buf, "  >  "); }
	void SendMes(const char *mes) const;
	void GetStr(char *HelpBuf);
	void GetOpt(const char *CmpOpt, char *HelpBuf);
};

struct AuctItem {
	int num, price, month, MaxPrice;
	AuctItem *next;
	AuctItem(int n ,int p, int m, int mp, AuctItem *a)
		: num(n), price(p), month(m), MaxPrice(mp), next(a) {}
};

class PlayerInfo {
	int number, active, money, fact, raw, prod, AutoFact;
	char name[NameSize];
	AuctItem *bought, *sold;
	int GetDif(AuctItem *item) const;
	int GetLastDif(AuctItem *item, int month) const;
	int GetLast(AuctItem *item, int month) const;
	void Delete(AuctItem *item);
public:
	PlayerInfo();
	~PlayerInfo() { Delete(bought); Delete(sold); }
	int CheckName(char *MyName) const { return !MyStrcmp(MyName, name); }
	int GetAfact() const { return AutoFact; }
	int GetFact() const { return fact; }
	int GetProd() const	{ return prod; }
	int GetRaw() const { return raw; }
	int GetMoney() const { return money; }
	int GetActive() const { return active; }
	int GetDifBought() const { return -GetDif(bought); }
	int GetDifSold() const { return GetDif(sold); }
	int GetLastDifBought(int month) const { return -GetLastDif(bought,month); }
	int GetLastDifSold(int month) const { return GetLastDif(sold,month); }
	int GetLastBought(int month) const { return GetLast(bought,month); }
	int GetLastSold(int month) const { return GetLast(sold,month); }
	const char *GetName() const { return name; }
	void Deactive() { active = 0; }
	void InitInfo(char *HelpBuf, int i);
	void SetInfo(char *HelpBuf);
	void AuctResBought(char *HelpBuf, int month, int MaxPrice);
	void AuctResSold(char *HelpBuf, int month, int MaxPrice);
	void PrintInfo();
};

class GameInfo {
	Socket sock;
	int month, MyNum, MaxNum;
	int raw, RawPrice, prod, ProdPrice;
	char *MyName;
	PlayerInfo *pInfo;
	void EnterGame(char **argv);
public:
	GameInfo(int argc, char **argv);
	~GameInfo() { delete [] pInfo; }
	void UpdateInfo();
	void PrintInfo();
	void WaitAuction();
	void SendProd(long long num) const;
	void SendSell(long long price, long long prod) const;
	void SendBuy(long long price, long long raw) const;
	void SendBuild() const;
	void SendAbuild() const;
	void SendUpgrade() const;
	void PrintMyInfo() const { pInfo[MyNum].PrintInfo(); }
	void PrintPlayerInfo(int i) const { pInfo[i-1].PrintInfo(); }
	int GetMinRawPrice() const { return RawPrice; }
	int GetMaxProdPrice() const { return ProdPrice; }
	int GetMaxRaw() const { return raw; }
	int GetMaxProd() const { return prod; }
	int GetMaxNum() const { return MaxNum; }
	int GetMonth() const { return month; }
	int GetMyRaw() const { return pInfo[MyNum].GetRaw(); }
	int GetMyProd() const { return pInfo[MyNum].GetProd(); }
	int GetMyFact() const { return pInfo[MyNum].GetFact(); }
	int GetMyMoney() const { return pInfo[MyNum].GetMoney(); }
	int GetMyAfact() const { return pInfo[MyNum].GetAfact(); }
	int GetMyNum() const { return MyNum+1; }
	int GetPlayerRaw(int i) const { return pInfo[i-1].GetRaw(); }
	int GetPlayerProd(int i) const { return pInfo[i-1].GetProd(); }
	int GetPlayerFact(int i) const { return pInfo[i-1].GetFact(); }
	int GetPlayerMoney(int i) const { return pInfo[i-1].GetMoney(); }
	int GetPlayerAfact(int i) const { return pInfo[i-1].GetAfact(); }
	int GetPlayerActive(int i) const { return pInfo[i-1].GetActive(); }
	int GetPlayerDifBought(int i) const
		{ return pInfo[i-1].GetDifBought(); }
	int GetPlayerDifSold(int i) const
		{ return pInfo[i-1].GetDifSold(); }
	int GetPlayerLastDifBought(int i) const
		{ return pInfo[i-1].GetLastDifBought(month); }
	int GetPlayerLastDifSold(int i) const
		{ return pInfo[i-1].GetLastDifSold(month); }
	int GetPlayerLastBought(int i) const
		{ return pInfo[i-1].GetLastBought(month); }
	int GetPlayerLastSold(int i) const
		{ return pInfo[i-1].GetLastSold(month); }
};

#endif
