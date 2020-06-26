#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "tables.h"

enum {MaxSize = 4096, NameSize = 20, OptSize = 20};
enum {IPAddress = 1, PortNum, Script, RobotName, Action, NumPlayersOrNumGame};
enum {NumArg = 7};

class Socket {
	int fd, BufSize;
	char buf[MaxSize+1];
	int SearchEndLine() const
	{
		for(int i = 0; i < BufSize; i++)
			if (buf[i] == '\n')
				return i;
		return -1;
	}
	void CheckEnd(char *HelpBuf) const
	{
		char option[OptSize] = "";
		if (HelpBuf[0] == '&') {
			sscanf(HelpBuf, "& %20s", option);
			if (!strcmp(option, "YOU_WIN"))
				throw "You won!\n";
		}
	}
public:
	Socket(int argc, char **argv)
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
	int CheckStart() const { return strcmp(buf, "  >  "); }
	void SendMes(const char *mes) const
	{
		write(fd, mes, strlen(mes));
		printf("%s", mes);
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
		CheckEnd(HelpBuf);
#ifdef DEBUGGING
		printf("//%s\n", HelpBuf);
#endif 
	}  
	void GetOpt(const char *CmpOpt, char *HelpBuf)
	{
		char option[OptSize] = "";
		do {
			GetStr(HelpBuf);
			if (HelpBuf[0] == '&')
				sscanf(HelpBuf, "& %20s", option);
		} while(strcmp(option, CmpOpt));
	}
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
	int GetDif(AuctItem *item) const
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
	int GetLast(AuctItem *item, int month) const
	{
		if (item == 0) {
			return 0;
		} else {
			if (item->month == month) {
				return item->price;
			} else {
				return 0;
			}
		}
	}
	void Delete(AuctItem *item)
	{
		AuctItem *help;
		while(item != 0) {
			help = item;
			item = item->next;
			delete help;
		}
	}
public:
	PlayerInfo()
	{
		number = active = money = fact = raw = prod = AutoFact = 0;
		name[0] = 0;
		bought = sold = 0;
	}
	~PlayerInfo() { Delete(bought); Delete(sold); }
	int CheckName(char *MyName) const { return !strcmp(MyName, name); }
	int GetAfact() const { return AutoFact; }
	int GetFact() const { return fact; }
	int GetProd() const	{ return prod; }
	int GetRaw() const { return raw; }
	int GetMoney() const { return money; }
	int GetActive() const { return active; }
	int GetDifBought() const { return -GetDif(bought); }
	int GetDifSold() const { return GetDif(sold); }
	int GetLastBought(int month) const { return GetLast(bought, month); }
	int GetLastSold(int month) const { return GetLast(sold, month); }
	const char *GetName() const { return name; }
	void Deactive() { active = 0; }
	void InitInfo(char *HelpBuf, int i);
	void SetInfo(char *HelpBuf);
	void AuctResBought(char *HelpBuf, int month, int MaxPrice);
	void AuctResSold(char *HelpBuf, int month, int MaxPrice);
	void PrintInfo();
};

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
	void SendMes(const char *mes) const { sock.SendMes(mes); }
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
	int GetPlayerLastBought(int i) const
		{ return pInfo[i-1].GetLastBought(month); }
	int GetPlayerLastSold(int i) const
		{ return pInfo[i-1].GetLastSold(month); }
};

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

class Lexeme {
	void *value;
	int LexNum, LineNum, TableIDNum, IdentType;
public:
	Lexeme(): value(NULL), LexNum(LexEmpty), LineNum(0) {}
	Lexeme(char buf[], int &BufSize, int line, int num);
	Lexeme(int line, int num);
	~Lexeme();
	int Empty() { return !LexNum; }
	int CheckCorrect() { return LexNum != LexError; } 
	int GetLexNum() { return LexNum; } 
	int GetLineNum() { return LineNum; }
	char *GetName() { return (char *)value; }
	long long GetInt() { return *((long long *)value); }
	double GetReal() { return *((double *)value); }
	char *GetString() { return (char *)value; }
	void Print(); 
};

Lexeme::Lexeme(char buf[], int &BufSize, int line, int num)
{
	LexNum = num;
	LineNum = line;
	if (LexNum == LexIdent || LexNum == LexError) {
		value = new char[BufSize + 1];
		for(int i = 0; i <= BufSize; i++)
			((char *)value)[i] = buf[i];
	} else if (LexNum == LexStr) {
		value = new char[BufSize - 1];
		for(int i = 0; i < BufSize - 2; i++)
			((char *)value)[i] = buf[i+1];
		((char *)value)[BufSize - 1] = 0;
	} else if (LexNum == LexValInt) {
		value = new long long(strtoll(buf, NULL, 10));
	} else if (LexNum == LexValReal) {
		value = new double(strtod(buf, NULL));
	}
	buf[0] = 0;
	BufSize = 0;
}

Lexeme::Lexeme(int line, int num)
{
	LexNum = num;
	LineNum = line;
}

void Lexeme::Print() 
{
	if (LexNum == LexIdent || LexNum == LexStr) {
		printf("%s", (char *)value);
	} else if (LexNum == LexValInt) {
		printf("%lld", *(long long *)value);
	} else if (LexNum == LexValReal) {
		printf("%f", *(double *)value);
	} else {
		printf("%s", TableOfWords[LexNum]);
	}
}

Lexeme::~Lexeme()
{
	if (LexNum == LexIdent || LexNum == LexStr)
		delete [] (char *)value;
	if (LexNum == LexValInt)
		delete (long long *)value;
	if (LexNum == LexValReal)
		delete (double *)value;
}

class Automat {
	enum {
		H, String, Ident, Int, Real, Equal, lg, 
		Comment, MultiAssign, Error, S, SResend
	};
	enum {MaxBuf = 4096};
	Lexeme *lex;
	char buf[MaxBuf];
	int BufSize, state, line;
	Lexeme *StateS(char c, int begin);
	void ChangeState(char c);
	void StateH(char c);
	void StateString(char c);
	void StateIdent(char c);
	void StateInt(char c);
	void StateReal(char c);
	void StateEqual(char c);
	void StateLessGreater(char c);
	void StateComment(char c);
	void StateMultiAssign(char c);
	void AddBuf(char c);
	int SearchPoint();
	int GetLexNum();
	int Letter(char c); 
	int Digit(char c);
	int EndLine(char c);
	int Brace(char c);
	int Compare(char c);
	int Single(char c);
	int Other(char c);
	int All(char c);
	int Delimiter(char c);
public:
	Automat(): lex(NULL), BufSize(0), state(H), line(1) { buf[0] = 0; }
	Lexeme *FeedChar(char c);
};

int Automat::Letter(char c) 
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int Automat::Digit(char c)
{
	return c >= '0' && c <= '9';
}

int Automat::EndLine(char c)
{
	return c == '\r' || c == '\n' || c == EOF;
}

int Automat::Brace(char c)
{
	return c=='(' || c==')' || c=='[' || c==']' || c=='{' || c=='}';
}

int Automat::Compare(char c)
{
	return c == '>' || c == '<' || c == '=';
}

int Automat::Single(char c)
{
	return c==':'||c==';'||c==','||c=='+'||c=='-'||c=='*'||c=='/'||c=='!';
}

int Automat::Other(char c)
{
	return c =='@'||c=='#'||c=='$'||c=='%'||c=='?'||c=='&';
}

int Automat::All(char c)
{
	return Letter(c)||Digit(c)||Brace(c)||Compare(c)||Single(c)||Other(c); 
}

int Automat::Delimiter(char c)
{
	int a = c == ' ' || c == '\t' || Digit(c);
	return EndLine(c)||Brace(c)||Compare(c)||Single(c)||c=='&'||c=='#'||a;
}

Lexeme *Automat::FeedChar(char c)
{
	if (state == S) {
		return StateS(c, 1);
	} else if (state == Error) {
		return new Lexeme(buf, BufSize, line, LexError);
	}
	ChangeState(c);
	if (state == S) {
		return StateS(c, 0);
	} else if (state == SResend) {
		return StateS(c, 1);
	} else if (state == Error) {
		return new Lexeme(buf, BufSize, line, LexError);
	} else {
		AddBuf(c);
	}
	return NULL;
}

Lexeme *Automat::StateS(char c, int begin)
{
	if (!begin)
		AddBuf(c);
	state = H;
	lex = new Lexeme(buf, BufSize, line, GetLexNum());
	if (c == '\n')
		line++;
	if (begin) {
		ChangeState(c);
		AddBuf(c);
	}
	if ((*lex).Empty()) {
		delete lex;
		if (c == EOF) {
			lex = new Lexeme(line, 0);
		} else {
			lex = NULL;
		}
	}
	return lex;
}

void Automat::ChangeState(char c)
{
	if (state == H) {
		StateH(c);
	} else if (state == String) {
		StateString(c);
	} else if (state == Ident) {
		StateIdent(c);
	} else if (state == Int) {
		StateInt(c);
	} else if (state == Real) {
		StateReal(c);
	} else if (state == Equal) {
		StateEqual(c);
	} else if (state == lg) {
		StateLessGreater(c);
	} else if (state == Comment) {
		StateComment(c);
	} else if (state == MultiAssign) {
		StateMultiAssign(c);
	}
}

void Automat::StateH(char c)
{
	if (Digit(c)) {
		state = Int;
	} else if (c == '\"') {
		state = String;
	} else if (Letter(c)) {
		state = Ident;
	} else if (c == '=') {
		state = Equal;
	} else if (c == '>' || c == '<') {
		state = lg;
	} else if (c == '\\') {
		state = Comment;
	} else if (c == ':') {
		state = MultiAssign;
	} else if (EndLine(c) || Brace(c) || Single(c) || c == ' ' || c == '\t') {
		state = S;
	} else {
		state = Error;
	}
}

void Automat::StateString(char c)
{
	if (c == '\"') {
		state = S;
	} else if (All(c) || c == ' ' || c == '\t') {
	} else {
		state = Error;
	}
}

void Automat::StateIdent(char c)
{
	if (Letter(c) || Digit(c)) {
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateInt(char c)
{
	if (Digit(c)) {
	} else if (c == '.') {
		state = Real;
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateReal(char c)
{
	if (Digit(c)) {
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateEqual(char c)
{
	if (c == '=' || c == '!') {
		state = S;
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateLessGreater(char c)
{
	if (c == '=') {
		state = S;
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateComment(char c)
{
	if (EndLine(c)) {
		state = S;
	}
}

void Automat::StateMultiAssign(char c)
{
	if (c == '=') {
		state = S;
	} else {
		state = Error;
	}
}

void Automat::AddBuf(char c)
{
	if (((c != ' ' && c != '\t') || BufSize != 0) && !EndLine(c)) {
		buf[BufSize++] = c;
		buf[BufSize] = 0;
	}
}

int Automat::SearchPoint()
{
	for(int i = 0; i < BufSize; i++)
		if (buf[i] == '.')
			return 1;
	return 0;
}

int Automat::GetLexNum()
{
	int i = 0;
	if (Digit(buf[0])) { 
		if (SearchPoint()) {
			return LexValReal;
		} else {
		return LexValInt;
		}
	} else if (buf[0] == '\\') {
		return LexEmpty;
	} else if (buf[0] == '"') {
		return LexStr;
	}
	while(TableOfWords[i] != NULL) {
		if (!strcmp(buf, TableOfWords[i]))
			return i;
		i++;
	}
	if (Letter(buf[0]))
		return LexIdent;
	return LexError;
}

class IpnElem;

enum {InFstDim, InScdDim, InVarDesc, InAssign, InAddFst, InAddScd,
	InSubFst, InSubScd, InMulFst, InMulScd, InDivFst, InDivScd,
	InOrFst, InOrScd, InAndFst, InAndScd, InLessFst, InLessScd,
	InLessEFst, InLessEScd, InGreatFst, InGreatScd,
	InGreatEFst, InGreatEScd, InEqualFst, InEqualScd,
	InNotEqualFst, InNotEqualScd, InNeg, InOpGoFalse, InPut,
	InSemicolon, InVar, InOpGo, InTakeValue
};

class IpnEx {
	int LineNum;
public:
	IpnEx(int l = 0): LineNum(l) {}
	virtual ~IpnEx() {}
	int GetLineNum() { return LineNum; }
	virtual void PrintError() const = 0;
	void HandleError() const
	{
		printf("Error in line %d: ", LineNum);
		PrintError();
	}
};

class IpnExNotInt: public IpnEx {
	int where;
public:
	IpnExNotInt(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotInt() {}
	virtual void PrintError() const
	{
		if (where == InFstDim) {
			printf("integer value expected ");
			printf("as first dimension of array ");
		} else if (where == InScdDim) {
			printf("integer value expected ");
			printf("as second dimension of array ");
		} else if (where == InVarDesc) {
			printf("integer value expected ");
			printf("as initial value in description of integer ");
		} else if (where == InAssign) {
			printf("expression after \"=\" is not integer type ");
		}
		printf("\n");
	}
};

class IpnExNotIntGameAct: public IpnEx {
public:
	IpnExNotIntGameAct(int l = 0): IpnEx(l) {}
	virtual ~IpnExNotIntGameAct() {}
	virtual void PrintError() const
		{ printf("operand in game action is not integer type\n"); }
};

class IpnExNotIntOrReal: public IpnEx {
	int where;
public:
	IpnExNotIntOrReal(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotIntOrReal() {}
	virtual void PrintError() const
	{
		if (where == InAssign) {
			printf("expression after \"=\" is not integer or real type ");
		} else if (where == InVarDesc) {
			printf("integer or real value expected ");
			printf("as initial value in description of real ");
		} else if (where == InAddFst) {
			printf("first operand in addition is not integer or real ");
		} else if (where == InAddScd) {
			printf("second operand in addition is not integer or real ");
		} else if (where == InSubFst) {
			printf("first operand in subtraction is not integer or real ");
		} else if (where == InSubScd) {
			printf("second operand in subtraction is not integer or real ");
		} else if (where == InMulFst) {
			printf("first operand in multiplication is not integer or real ");
		} else if (where == InMulScd) {
			printf("second operand in multiplication is not integer or real ");
		} else if (where == InDivFst) {
			printf("first operand in division is not integer or real ");
		} else if (where == InDivScd) {
			printf("second operand in division is not integer or real ");
		} else if (where == InOrFst) {
			printf("first operand in \"or\" is not integer or real ");
		} else if (where == InOrScd) {
			printf("second operand in \"or\" is not integer or real ");
		} else if (where == InAndFst) {
			printf("first operand in \"and\" is not integer or real ");
		} else if (where == InAndScd) {
			printf("second operand in \"and\" is not integer or real ");
		} else if (where == InLessFst) {
			printf("first operand in \"<\" is not integer or real ");
		} else if (where == InLessScd) {
			printf("second operand in \"<\" is not integer or real ");
		} else if (where == InLessEFst) {
			printf("first operand in \"<=\" is not integer or real ");
		} else if (where == InLessEScd) {
			printf("second operand in \"<=\" is not integer or real ");
		} else if (where == InGreatFst) {
			printf("first operand in \">\" is not integer or real ");
		} else if (where == InGreatScd) {
			printf("second operand in \">\" is not integer or real ");
		} else if (where == InGreatEFst) {
			printf("first operand in \">=\" is not integer or real ");
		} else if (where == InGreatEScd) {
			printf("second operand in \">=\" is not integer or real ");
		} else if (where == InEqualFst) {
			printf("first operand in \"==\" is not integer or real ");
		} else if (where == InEqualScd) {
			printf("second operand in \"==\" is not integer or real ");
		} else if (where == InNotEqualFst) {
			printf("first operand in \"=!\" is not integer or real ");
		} else if (where == InNotEqualScd) {
			printf("second operand in \"=!\" is not integer or real ");
		} else if (where == InNeg) {
			printf("operand in \"!\" is not integer or real ");
		} else if (where == InOpGoFalse) {
			printf("operand in conditional jump is not integer or real ");
		}
		printf("\n");
	}
};

class IpnExNotStr: public IpnEx {
	int where;
public:
	IpnExNotStr(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotStr() {}
	virtual void PrintError() const
	{
		if (where == InAssign) {
			printf("expression after \"=\" is not string type ");
		} else if (where == InVarDesc) {
			printf("string value expected ");
			printf("as initial value in description of string ");
		}
		printf("\n");
	}
};

class IpnExNotIntOrRealOrString: public IpnEx {
	int where;
public:
	IpnExNotIntOrRealOrString(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotIntOrRealOrString() {}
	virtual void PrintError() const
	{
		if (where == InPut) {
			printf("integer or real or string value expected ");
			printf("as argument of put ");
		} else if (where == InSemicolon) {
			printf("integer or real or string value expected before \";\" ");
		}
		printf("\n");
	}
};

class IpnExNotFound: public IpnEx {
	int where;
	char *name;
public:
	IpnExNotFound(int l, int w, char *n): IpnEx(l), where(w)
		{ name = strdup(n); }
	virtual ~IpnExNotFound() { delete [] name; }
	virtual void PrintError() const
	{
		if (where == InVar || where == InAssign) {
			printf("variable \"%s\" was not declared in this scope ", name);
		}
		printf("\n");
	}
};

class IpnExRedec: public IpnEx {
	int where;
	char *name;
public:
	IpnExRedec(int l, int w, char *n): IpnEx(l), where(w)
		{ name = strdup(n); }
	virtual ~IpnExRedec() { delete [] name; }
	virtual void PrintError() const
	{
		if (where == InVarDesc) {
			printf("variable \"%s\" is already declared ", name);
		}
		printf("\n");
	}
};

class IpnExStackEmpty: public IpnEx {
public:
	IpnExStackEmpty(int l): IpnEx(l) {}
	virtual ~IpnExStackEmpty() {}
	virtual void PrintError() const { printf("Stack is empty\n"); }
};

class IpnExSegFault: public IpnEx {
	char *name;
public:
	IpnExSegFault(int l): IpnEx(l), name(0) {}
	IpnExSegFault(int l, char *n): IpnEx(l) { name = strdup(n); }
	virtual ~IpnExSegFault()
	{
		if (name)
			delete [] name;
	}
	virtual void PrintError() const
	{
		if (name) {
			printf("%s: Segmentation fault\n", name);
		} else {
			printf("Segmentation fault\n");
		}
	}
};

class IpnExNotLabel: public IpnEx {
	int where;
public:
	IpnExNotLabel(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotLabel() {}
	virtual void PrintError() const
	{
		if (where == InOpGo) {
			printf("operand in unconditional jump is not label ");
		} else if (where == InOpGoFalse) {
			printf("operand in conditional jump is not label ");
		}
		printf("\n");
	}
};

class IpnExNotVarAddr: public IpnEx {
	int where;
public:
	IpnExNotVarAddr(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotVarAddr() {}
	virtual void PrintError() const
	{
		if (where == InAssign) {
			printf("operand in assignment is not address of variable ");
		} else if (where == InTakeValue) {
			printf("operand in taking of value is not address of variable ");
		}
		printf("\n");
	}
};

enum {NotFound, Int, Real, String};

struct VarElem {
	int type, FstDim, ScdDim, level;
	char *name;
	void *value;
	VarElem *next;
	VarElem(char *n, int t, void *v, int f, int s, int l)
		: type(t), FstDim(f), ScdDim(s), level(l), name(n), value(v) {}
};

struct IpnItem {
	IpnElem *elem;
	IpnItem *next;
	IpnItem() {}
	IpnItem(IpnElem *e, IpnItem *n): elem(e), next(n) {}
};

class IpnElem {
	int LineNum;
public:
	IpnElem(int l = 0): LineNum(l) {}
	virtual ~IpnElem() {}
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const = 0;
	int GetLineNum() const { return LineNum; }
	virtual void Print() const = 0;
protected:
	void Push(IpnItem **stack, IpnElem *elem) const
	{
		*stack = new IpnItem(elem, *stack);
	}
	IpnElem *Pop(IpnItem **stack) const
	{
		IpnElem *elem;
		IpnItem *help;
		if (*stack == 0) {
			throw new IpnExStackEmpty(GetLineNum());
		} else {
			help = *stack;
			elem = (*stack)->elem;
			*stack = (*stack)->next;
			delete help;
		}
		return elem;
	}
};

class IpnConst: public IpnElem {
public:
	IpnConst(int l = 0): IpnElem(l) {}
	virtual IpnElem *Clone() const = 0;
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const
	{
		Push(stack, Clone());
		*CurCmd = (*CurCmd)->next;
	}
};

class IpnInt: public IpnConst {
	long long value;
public:
	IpnInt(const long long v, int l = 0): IpnConst(l), value(v) {}
	virtual ~IpnInt() {}
	virtual IpnElem *Clone() const
		{ return new IpnInt(value, GetLineNum()); }
	long long Get() const { return value; }
	void Print() const { printf("IpnInt %lld\n", value); }
};

class IpnReal: public IpnConst {
	double value;
public:
	IpnReal(const double v, int l = 0): IpnConst(l), value(v) {}
	virtual ~IpnReal() {}
	virtual IpnElem *Clone() const
		{ return new IpnReal(value, GetLineNum()); }
	double Get() const { return value; }
	void Print() const { printf("IpnReal %f\n", value); }
};

class IpnLabel: public IpnConst {
	IpnItem *value;
public:
	IpnLabel(IpnItem *v, int l = 0): IpnConst(l), value(v) {}
	virtual ~IpnLabel() {}
	virtual IpnElem *Clone() const
		{ return new IpnLabel(value, GetLineNum()); }
	IpnItem *Get() const { return value; }
	void Print() const { printf("IpnLabel "); value->elem->Print(); }
};

class IpnEndOfArg: public IpnConst {
public:
	IpnEndOfArg(int l = 0): IpnConst(l) {}
	virtual ~IpnEndOfArg() {}
	virtual IpnEndOfArg *Clone() const
		{ return new IpnEndOfArg(GetLineNum()); }
	void Print() const { printf("IpnEndOfArg\n"); }
};

class IpnString: public IpnConst {
	char *str;
public:
	IpnString(const char *s, int l = 0): IpnConst(l) { str = strdup(s); }
	IpnString(const IpnString &other) { str = strdup(other.str); }
	virtual ~IpnString() { delete [] str; }
	virtual IpnString *Clone() const
		{ return new IpnString(str, GetLineNum()); }
	char *Get() const { return str; }
	void Print() const { printf("IpnString %s\n", str); }
};

class IpnVarAddr: public IpnConst {
	char *str;
public:
	IpnVarAddr(const char *s, int l = 0): IpnConst(l) { str = strdup(s); }
	IpnVarAddr(const IpnVarAddr &other) { str = strdup(other.str); }
	virtual ~IpnVarAddr() { delete [] str; }
	virtual IpnVarAddr *Clone() const
		{ return new IpnVarAddr(str, GetLineNum()); }
	char *Get() const { return str; }
	void Print() const { printf("IpnVarAddr %s\n", str); }
};

class IpnVarOrAssign: public IpnElem {
protected:
	int GetType(VarElem **VarList, char *search) const
	{
		VarElem *cur = *VarList;
		while(cur != 0) {
			if (!strcmp(cur->name, search))
				return cur->type;
			cur = cur->next;
		}
		return NotFound;
	}
	void GetDims(IpnItem **stack, int &FstDim, int &ScdDim, int inDesc) const
	{
		IpnElem *operand2 = Pop(stack), *operand1 = Pop(stack);
		IpnInt *iInt2 = dynamic_cast<IpnInt*>(operand2);
		IpnInt *iInt1 = dynamic_cast<IpnInt*>(operand1);
		if (!iInt2) {
			throw new IpnExNotInt(operand2->GetLineNum(), InScdDim);
		} else if (!iInt1) {
			throw new IpnExNotInt(operand1->GetLineNum(), InScdDim);
		} else {
			ScdDim = iInt2->Get();
			FstDim = iInt1->Get();
		}
		delete operand1;
		delete operand2;
		if (FstDim < 0 || ScdDim < 0)
			throw new IpnExSegFault(GetLineNum());
		if (inDesc) {
			if (FstDim == 0 || ScdDim == 0)
				throw new IpnExSegFault(GetLineNum());
		}
	}
public:
	IpnVarOrAssign(int l = 0): IpnElem(l) {}
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const = 0;
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const
	{
		IpnElem *res = EvaluateFun(stack, VarList);
		if (res)
			Push(stack, res);
		*CurCmd = (*CurCmd)->next;
	}
};

class IpnTakeValue: public IpnVarOrAssign {
	template <class T>
	T GetValue(VarElem **VarList, int FstDim, int ScdDim, char *name) const
	{
		VarElem *cur = *VarList;
		while(cur != 0) {
			if (!strcmp(cur->name, name)) {
				if (FstDim >= cur->FstDim || ScdDim >= cur->ScdDim) {
					throw new IpnExSegFault(GetLineNum(), name);
				} else {
					int dim = FstDim*(cur->ScdDim) + ScdDim;
					delete [] name;
					return ((T *)(cur->value))[dim];
				}
			}
			cur = cur->next;
		}
		return 0;
	}
	char *GetString(VarElem **VarList, char *name) const
	{
		VarElem *cur = *VarList;
		while(cur != 0) {
			if (!strcmp(cur->name, name)) {
				delete [] name;
				return (char *)(cur->value);
			}
			cur = cur->next;
		}
		return 0;
	}
public:
	IpnTakeValue(int l = 0): IpnVarOrAssign(l) {}
	virtual ~IpnTakeValue() {}
	void Print() const { printf("IpnTakeValue\n"); }
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const
	{
		IpnElem *operand = Pop(stack);
		IpnVarAddr *iVarAddr = dynamic_cast<IpnVarAddr*>(operand);
		if (!iVarAddr)
			throw new IpnExNotVarAddr(operand->GetLineNum(), InTakeValue);
		char *name = strdup(iVarAddr->Get());
		delete operand;
		int FstDim, ScdDim;
		GetDims(stack, FstDim, ScdDim, 0);
		int type = GetType(VarList, name);
		if (type == NotFound) {
			throw new IpnExNotFound(GetLineNum(), InVar, name);
		} else if (type == Int) {
			long long res = GetValue<long long>(VarList, FstDim, ScdDim, name);
			return new IpnInt(res, GetLineNum());
		} else if (type == Real) {
			double res = GetValue<double>(VarList, FstDim, ScdDim, name);
			return new IpnReal(res, GetLineNum());
		} else {
			return new IpnString(GetString(VarList, name), GetLineNum());
		}
	}
};

class IpnAssign: public IpnVarOrAssign {
	template <class T, class IpnType>
	void SetValue(IpnItem **stack, VarElem **VarList, IpnVarAddr *VarAddr,
		IpnType IpnName) const
	{
		int FstDim, ScdDim;
		VarElem *cur = *VarList;
		char *name = VarAddr->Get();
		GetDims(stack, FstDim, ScdDim, 0);
		while(cur != 0) {
			if (!strcmp(cur->name, name)) {
				if (FstDim >= cur->FstDim || ScdDim >= cur->ScdDim) {
					throw new IpnExSegFault(GetLineNum(), name);
				} else {
					int dim = FstDim*(cur->ScdDim) + ScdDim;
					((T *)(cur->value))[dim] = IpnName->Get();
					delete VarAddr;
					delete IpnName;
					return;
				}
			}
			cur = cur->next;
		}
	}
	void SetString(VarElem **VarList, IpnVarAddr *VarAddr,
		IpnString *iString) const
	{
		VarElem *cur = *VarList;
		char *name = VarAddr->Get();
		while(cur != 0) {
			if (!strcmp(cur->name, name)) {
				delete [] (char *)(cur->value);
				cur->value = strdup(iString->Get());
				delete VarAddr;
				delete iString;
				return;
			}
			cur = cur->next;
		}
	}
public:
	void Print() const { printf("IpnAssign\n"); }
	IpnAssign(int l = 0): IpnVarOrAssign(l) {}
	virtual ~IpnAssign() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const
	{
		IpnElem *operand2 = Pop(stack), *operand1 = Pop(stack);
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand2);
		IpnReal *iReal = dynamic_cast<IpnReal*>(operand2);
		IpnString *iString = dynamic_cast<IpnString*>(operand2);
		IpnVarAddr *iVarAddr = dynamic_cast<IpnVarAddr*>(operand1);
		if (!iVarAddr)
			throw new IpnExNotVarAddr(operand1->GetLineNum(), InAssign);
		int type = GetType(VarList, iVarAddr->Get());
		if (type == NotFound) {
			int line = iVarAddr->GetLineNum();
			throw new IpnExNotFound(line, InAssign, iVarAddr->Get());
		} else if (type == Real) {
			if (iInt) {
				SetValue<double, IpnInt *>(stack, VarList, iVarAddr, iInt);
				return new IpnInt(iInt->Get(), GetLineNum());
			} else if (iReal) {
				SetValue<double, IpnReal *>(stack, VarList, iVarAddr, iReal);
				return new IpnReal(iReal->Get(), GetLineNum());
			} else {
				throw new IpnExNotIntOrReal(operand2->GetLineNum(), InAssign);
			}
		} else if (type == Int) {
			if (iInt) {
				SetValue<long long, IpnInt *>(stack, VarList, iVarAddr, iInt);
				return new IpnInt(iInt->Get(), GetLineNum());
			} else {
				throw new IpnExNotInt(operand2->GetLineNum(), InAssign);
			}
		} else if (type == String) {
			if (iString) {
				SetString(VarList, iVarAddr, iString);
				return new IpnString(iString->Get(), GetLineNum());
			} else {
				throw new IpnExNotStr(operand2->GetLineNum(), InAssign);
			}
		}
		return 0;
	}
};

class IpnVarDesc: public IpnVarOrAssign {
	int type, FstDim, ScdDim, considered;
	char *name;
	void *value;
	int SearchName(VarElem *VarList)
	{
		while(VarList != 0) {
			if (!strcmp(VarList->name, name))
				return 1;
			VarList = VarList->next;
		}
		return 0;
	}
	void AddInVarList(VarElem **VarList, int level)
	{
		VarElem *NewElem;
		NewElem = new VarElem(name, type, value, FstDim, ScdDim, level);
		NewElem->next = *VarList;
		*VarList = NewElem;
		considered = 1;
	}
public:
	void Print() const { printf("IpnVarDesc %s\n", name); }
	IpnVarDesc(char *n, int t, int l = 0)
		: IpnVarOrAssign(l), type(t), FstDim(0), ScdDim(0), 
		considered(0), value(0)
	{
		name = strdup(n);
	}
	~IpnVarDesc()
	{
		delete [] name;
		if (type == Int) {
			delete [] (long long *)value;
		} else if (type == Real) {
			delete [] (double *)value;
		} else if (type == String) {
			delete [] (char *)value;
		}
	}
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const
		{ return 0; }
	void Clear() { considered = 0; }
	void AddVar(IpnItem **stack, VarElem **VarList, int level)
	{
		IpnElem *operand3 = Pop(stack);
		GetDims(stack, FstDim, ScdDim, 1);
		if (!considered) {
			if (type == Int) {
				IpnInt *iInt = dynamic_cast<IpnInt*>(operand3);
				if (!iInt)
					throw new IpnExNotInt(operand3->GetLineNum(), InVarDesc);
				value = new long long[FstDim * ScdDim];
				for(int i = 0; i < FstDim * ScdDim; i++)
					((long long *)value)[i] = iInt->Get();
			} else if (type == Real) {
				IpnInt *iInt = dynamic_cast<IpnInt*>(operand3);
				IpnReal *iReal = dynamic_cast<IpnReal*>(operand3);
				double InitValue;
				if (iInt) {
					InitValue = iInt->Get();
				} else if (iReal) {
					InitValue = iReal->Get();
				} else {
					int line = operand3->GetLineNum();
					throw new IpnExNotIntOrReal(line, InVarDesc);
				}
				value = new double[FstDim * ScdDim];
				for(int i = 0; i < FstDim * ScdDim; i++)
					((double *)value)[i] = InitValue;
			} else if (type == String) {
				IpnString *iString = dynamic_cast<IpnString*>(operand3);
				if (!iString) {
					int line = operand3->GetLineNum();
					throw new IpnExNotStr(line, InVar);
				}
				value = strdup(iString->Get());
			}
			if (SearchName(*VarList))
				throw new IpnExRedec(GetLineNum(), InVarDesc, name);
			AddInVarList(VarList, level);
		}
		delete operand3;
	}
};

class IpnFunction: public IpnElem {
public:
	IpnFunction(int l = 0): IpnElem(l) {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const = 0;
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const
	{
		IpnElem *res = EvaluateFun(stack);
		if (res)
			Push(stack, res);
		*CurCmd = (*CurCmd)->next;
	}
};

class IpnArithmeticOperation: public IpnFunction {
public:
	IpnArithmeticOperation(int l = 0): IpnFunction(l) {}
	virtual ~IpnArithmeticOperation() {}
	virtual long long CalcInt(long long x, long long y) const = 0;
	virtual double CalcReal(double x, double y) const = 0;
	virtual void ThrowFst(int LineNum) const = 0;
	virtual void ThrowScd(int LineNum) const = 0;
	virtual IpnElem *EvaluateFun(IpnItem **stack) const
	{
		IpnElem *operand2 = Pop(stack), *operand1 = Pop(stack);
		long long OneIsReal = 0, ResInt;
		double ResReal;
		IpnInt *iInt1 = dynamic_cast<IpnInt*>(operand1);
		IpnInt *iInt2 = dynamic_cast<IpnInt*>(operand2);
		IpnReal *iReal1 = dynamic_cast<IpnReal*>(operand1);
		IpnReal *iReal2 = dynamic_cast<IpnReal*>(operand2);
		if (iReal1 || iReal2) {
			OneIsReal = 1;
			if (iReal1 && iReal2) {
				ResReal = CalcReal(iReal1->Get(), iReal2->Get());
			} else if (iReal1) {
				if (iInt2) {
					ResReal = CalcReal(iReal1->Get(), iInt2->Get());
				} else {
					ThrowScd(operand2->GetLineNum());
				}
			} else if (iReal2) {
				if (iInt1) {
					ResReal = CalcReal(iInt1->Get(), iReal2->Get());
				} else {
					ThrowFst(operand1->GetLineNum());
				}
			}
		} else if (iInt1 && iInt2) {
			ResInt = CalcInt(iInt1->Get(), iInt2->Get());
		} else if (!iInt1) {
			ThrowFst(operand1->GetLineNum());
		} else {
			ThrowScd(operand2->GetLineNum());
		}
		delete operand1;
		delete operand2;
		if (OneIsReal) {
			return new IpnReal(ResReal, GetLineNum());
		} else {
			return new IpnInt(ResInt, GetLineNum());
		}
	}
};

class IpnAdd: public IpnArithmeticOperation {
public:
	IpnAdd(int l = 0): IpnArithmeticOperation(l) {}
	virtual ~IpnAdd() {}
	void Print() const { printf("IpnAdd\n"); }
	long long CalcInt(long long x, long long y) const { return x + y; }
	double CalcReal(double x, double y) const { return x + y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InAddFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InAddScd); }
};

class IpnSub: public IpnArithmeticOperation {
public:
	IpnSub(int l = 0): IpnArithmeticOperation(l) {}
	virtual ~IpnSub() {}
	void Print() const { printf("IpnSub\n"); }
	long long CalcInt(long long x, long long y) const { return x - y; }
	double CalcReal(double x, double y) const { return x - y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InSubFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InSubScd); }
};

class IpnMul: public IpnArithmeticOperation {
public:
	IpnMul(int l = 0): IpnArithmeticOperation(l) {}
	virtual ~IpnMul() {}
	void Print() const { printf("IpnMul\n"); }
	long long CalcInt(long long x, long long y) const { return x * y; }
	double CalcReal(double x, double y) const { return x * y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InMulFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InMulScd); }
};

class IpnDiv: public IpnArithmeticOperation {
public:
	IpnDiv(int l = 0): IpnArithmeticOperation(l) {}
	virtual ~IpnDiv() {}
	void Print() const { printf("IpnDiv\n"); }
	long long CalcInt(long long x, long long y) const { return x / y; }
	double CalcReal(double x, double y) const { return x / y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InDivFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InDivScd); }
};

class IpnLogicalOperation: public IpnFunction {
public:
	IpnLogicalOperation(int l = 0): IpnFunction(l) {}
	virtual ~IpnLogicalOperation() {}
	virtual long long Calc(double x, double y) const = 0;
	virtual void ThrowFst(int LineNum) const = 0;
	virtual void ThrowScd(int LineNum) const = 0;
	virtual IpnElem *EvaluateFun(IpnItem **stack) const
	{
		IpnElem *operand2 = Pop(stack);
		long long ret;
		double res;
		IpnInt *iInt2 = dynamic_cast<IpnInt*>(operand2);
		IpnReal *iReal2 = dynamic_cast<IpnReal*>(operand2);
 		if (iReal2 || iInt2) {
			if (iReal2) {
				res = iReal2->Get();
			} else {
				res = iInt2->Get();
			}
		} else {
			ThrowScd(operand2->GetLineNum());
		}
		delete operand2;
		IpnElem *operand1 = Pop(stack);
		IpnInt *iInt1 = dynamic_cast<IpnInt*>(operand1);
		IpnReal *iReal1 = dynamic_cast<IpnReal*>(operand1);
 		if (iReal1 || iInt1) {
			if (iReal1) {
				ret = Calc(iReal1->Get(), res);
			} else {
				ret = Calc(iInt1->Get(), res);
			}
		} else {
			ThrowFst(operand1->GetLineNum());
		}
		delete operand1;
		return new IpnInt(ret, GetLineNum());
	}
};

class IpnOr: public IpnLogicalOperation {
public:
	IpnOr(int l = 0): IpnLogicalOperation(l) {}
	virtual ~IpnOr() {}
	void Print() const { printf("IpnOr\n"); }
	long long Calc(double x, double y) const { return x || y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InOrFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InOrScd); }
};

class IpnAnd: public IpnLogicalOperation {
public:
	IpnAnd(int l = 0): IpnLogicalOperation(l) {}
	virtual ~IpnAnd() {}
	void Print() const { printf("IpnAnd\n"); }
	long long Calc(double x, double y) const { return x && y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InAndFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InAndScd); }
};

class IpnLess: public IpnLogicalOperation {
public:
	IpnLess(int l = 0): IpnLogicalOperation(l) {}
	virtual ~IpnLess() {}
	void Print() const { printf("IpnLess\n"); }
	long long Calc(double x, double y) const { return x < y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InLessFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InLessScd); }
};

class IpnLessE: public IpnLogicalOperation {
public:
	IpnLessE(int l = 0): IpnLogicalOperation(l) {}
	virtual ~IpnLessE() {}
	void Print() const { printf("IpnLessE\n"); }
	long long Calc(double x, double y) const { return x <= y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InLessEFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InLessEScd); }
};

class IpnGreat: public IpnLogicalOperation {
public:
	IpnGreat(int l = 0): IpnLogicalOperation(l) {}
	virtual ~IpnGreat() {}
	void Print() const { printf("IpnGreat\n"); }
	long long Calc(double x, double y) const { return x > y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InGreatFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InGreatScd); }
};

class IpnGreatE: public IpnLogicalOperation {
public:
	IpnGreatE(int l = 0): IpnLogicalOperation(l) {}
	virtual ~IpnGreatE() {}
	void Print() const { printf("IpnGreatE\n"); }
	long long Calc(double x, double y) const { return x >= y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InGreatEFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InGreatEScd); }
};

class IpnEqual: public IpnLogicalOperation {
public:
	IpnEqual(int l = 0): IpnLogicalOperation(l) {}
	virtual ~IpnEqual() {}
	void Print() const { printf("IpnEqual\n"); }
	long long Calc(double x, double y) const { return x == y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InEqualFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InEqualScd); }
};

class IpnNotEqual: public IpnLogicalOperation {
public:
	IpnNotEqual(int l = 0): IpnLogicalOperation(l) {}
	virtual ~IpnNotEqual() {}
	void Print() const { printf("IpnNotEqual\n"); }
	long long Calc(double x, double y) const { return x != y; }
	virtual void ThrowFst(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InNotEqualFst); }
	virtual void ThrowScd(int LineNum) const 
		{ throw new IpnExNotIntOrReal(LineNum, InNotEqualScd); }
};

class IpnNeg: public IpnFunction {
public:
	void Print() const { printf("IpnNeg\n"); }
	IpnNeg(int l = 0): IpnFunction(l) {}
	virtual ~IpnNeg() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const
	{
		IpnElem *operand = Pop(stack);
		long long res;
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand);
		IpnReal *iReal = dynamic_cast<IpnReal*>(operand);
 		if (iReal || iInt) {
			if (iReal) {
				res = !(iReal->Get());
			} else {
				res = !(iInt->Get());
			}
		} else {
			throw new IpnExNotIntOrReal(operand->GetLineNum(), InNeg);
		}
		delete operand;
		return new IpnInt(res, GetLineNum());
	}
};

class IpnPut: public IpnFunction {
public:
	void Print() const { printf("IpnPut\n"); }
	IpnPut(int l = 0): IpnFunction(l) {}
	virtual ~IpnPut() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const
	{
		IpnItem *HelpList = 0, *HelpItem;
		IpnElem *operand = Pop(stack);
		while(!dynamic_cast<IpnEndOfArg*>(operand)) {
			HelpList = new IpnItem(operand, HelpList);
			operand = Pop(stack);
		}
		delete operand;
		while(HelpList != 0) {
			operand = HelpList->elem;
			HelpItem = HelpList;
			HelpList = HelpList->next;
			delete HelpItem;
			IpnInt *iInt = dynamic_cast<IpnInt*>(operand);
			IpnReal *iReal = dynamic_cast<IpnReal*>(operand);
			IpnString *iString = dynamic_cast<IpnString*>(operand);
			if (iInt) {
				printf("%lld ", iInt->Get());
			} else if (iReal) {
				printf("%f ", iReal->Get());
			} else if (iString) {
				printf("%s ", iString->Get());
			} else {
				int line = operand->GetLineNum();
				throw new IpnExNotIntOrRealOrString(line, InPut);
			}
			delete operand;
		}
		printf("\n");
		return 0;
	}
};

class IpnSemicolon: public IpnFunction {
public:
	void Print() const { printf("IpnSemicolon\n"); }
	IpnSemicolon(int l = 0): IpnFunction(l) {}
	virtual ~IpnSemicolon() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const
	{
		IpnElem *operand = Pop(stack);
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand);
		IpnReal *iReal = dynamic_cast<IpnReal*>(operand);
		IpnString *iString = dynamic_cast<IpnString*>(operand);
 		if (!iReal && !iInt && !iString) {
			int line = operand->GetLineNum();
			throw new IpnExNotIntOrRealOrString(line, InSemicolon);
		}
		delete operand;
		return 0;
	}
};

class IpnNoOp: public IpnFunction {
public:
	void Print() const { printf("IpnNoOp\n"); }
	IpnNoOp(int l = 0): IpnFunction(l) {}
	virtual ~IpnNoOp() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const { return 0; }
};

class IpnBraceL: public IpnFunction {
public:
	void Print() const { printf("IpnBraceL\n"); }
	IpnBraceL(int l = 0): IpnFunction(l) {}
	virtual ~IpnBraceL() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const { return 0; }
};

class IpnBraceR: public IpnFunction {
public:
	void Print() const { printf("IpnBraceR\n"); }
	IpnBraceR(int l = 0): IpnFunction(l) {}
	virtual ~IpnBraceR() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const { return 0; }
	void DelVar(VarElem **VarList, int &NestingLevel)
	{
		VarElem *help;
		while(*VarList != 0 && (*VarList)->level == NestingLevel) {
			help = *VarList;
			*VarList = (*VarList)->next;
			delete help;
		}
		NestingLevel--;
	}
};

class IpnGameAction: public IpnElem {
protected:
	long long GetInt(IpnItem **stack) const
	{
		IpnElem *operand = Pop(stack);
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand);
		if (iInt) {
			return  iInt->Get();
		} else {
			throw new IpnExNotIntGameAct(operand->GetLineNum());
		}
		return 0;
	}
	long long GetIntCheck(IpnItem **stack, GameInfo *gInfo) const
	{
		IpnElem *operand = Pop(stack);
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand);
		if (iInt) {
			long long num = iInt->Get();
			if (num >= 1 && num <= gInfo->GetMaxNum()) {
				return  num;
			} else {
				throw new IpnExSegFault(operand->GetLineNum());
			}
		} else {
			throw new IpnExNotIntGameAct(operand->GetLineNum());
		}
		return 0;
	}
public:
	IpnGameAction(int l = 0): IpnElem(l) {}
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const = 0;
	void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const
	{
		IpnElem *res = MakeAction(stack, gInfo);
		if (res)
			Push(stack, res);
		*CurCmd = (*CurCmd)->next;
	}
};

class IpnProd: public IpnGameAction {
public:
	IpnProd(int l): IpnGameAction(l) {}
	virtual ~IpnProd() {}
	virtual void Print() const { printf("IpnProd\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		char HelpBuf[MaxSize];
		sprintf(HelpBuf, "prod %lld\n", GetInt(stack));
		gInfo->SendMes(HelpBuf);
		return 0;
	}
};

class IpnSell: public IpnGameAction {
public:
	IpnSell(int l): IpnGameAction(l) {}
	virtual ~IpnSell() {}
	virtual void Print() const { printf("IpnSell\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long price = GetInt(stack), prod = GetInt(stack);
		char HelpBuf[MaxSize];
		sprintf(HelpBuf, "sell %lld %lld\n", prod, price);
		gInfo->SendMes(HelpBuf);
		return 0;
	}
};

class IpnBuy: public IpnGameAction {
public:
	IpnBuy(int l): IpnGameAction(l) {}
	virtual ~IpnBuy() {}
	virtual void Print() const { printf("IpnBuy\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long price = GetInt(stack), raw = GetInt(stack);
		char HelpBuf[MaxSize];
		sprintf(HelpBuf, "buy %lld %lld\n", raw, price);
		gInfo->SendMes(HelpBuf);
		return 0;
	}
};

class IpnBuild: public IpnGameAction {
public:
	IpnBuild(int l): IpnGameAction(l) {}
	virtual ~IpnBuild() {}
	virtual void Print() const { printf("IpnBuild\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		char HelpBuf[MaxSize];
		sprintf(HelpBuf, "build\n");
		gInfo->SendMes(HelpBuf);
		return 0;
	}
};

class IpnAbuild: public IpnGameAction {
public:
	IpnAbuild(int l): IpnGameAction(l) {}
	virtual ~IpnAbuild() {}
	virtual void Print() const { printf("IpnAbuild\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		char HelpBuf[MaxSize];
		sprintf(HelpBuf, "abuild\n");
		gInfo->SendMes(HelpBuf);
		return 0;
	}
};

class IpnUpgrade: public IpnGameAction {
public:
	IpnUpgrade(int l): IpnGameAction(l) {}
	virtual ~IpnUpgrade() {}
	virtual void Print() const { printf("IpnUpgrade\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		char HelpBuf[MaxSize];
		sprintf(HelpBuf, "upgrade\n");
		gInfo->SendMes(HelpBuf);
		return 0;
	}
};

class IpnPrintMyInfo: public IpnGameAction {
public:
	IpnPrintMyInfo(int l): IpnGameAction(l) {}
	virtual ~IpnPrintMyInfo() {}
	virtual void Print() const { printf("IpnPrintMyInfo\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		gInfo->PrintMyInfo();
		return 0;
	}
};

class IpnPrintMarketInfo: public IpnGameAction {
public:
	IpnPrintMarketInfo(int l): IpnGameAction(l) {}
	virtual ~IpnPrintMarketInfo() {}
	virtual void Print() const { printf("IpnPrintMarketInfo\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		gInfo->PrintInfo();
		return 0;
	}
};

class IpnPrintPlayerInfo: public IpnGameAction {
public:
	IpnPrintPlayerInfo(int l): IpnGameAction(l) {}
	virtual ~IpnPrintPlayerInfo() {}
	virtual void Print() const { printf("IpnPrintPlayerInfo\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		gInfo->PrintPlayerInfo(GetIntCheck(stack, gInfo));
		return 0;
	}
};

class IpnPlayerProd: public IpnGameAction {
public:
	IpnPlayerProd(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerProd() {}
	virtual void Print() const { printf("IpnPlayerProd\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerProd(num), GetLineNum());
	}
};

class IpnPlayerRaw: public IpnGameAction {
public:
	IpnPlayerRaw(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerRaw() {}
	virtual void Print() const { printf("IpnPlayerRaw\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerRaw(num), GetLineNum());
	}
};

class IpnPlayerFact: public IpnGameAction {
public:
	IpnPlayerFact(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerFact() {}
	virtual void Print() const { printf("IpnPlayerFact\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerFact(num), GetLineNum());
	}
};

class IpnPlayerMoney: public IpnGameAction {
public:
	IpnPlayerMoney(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerMoney() {}
	virtual void Print() const { printf("IpnPlayerMoney\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerMoney(num), GetLineNum());
	}
};

class IpnPlayerAfact: public IpnGameAction {
public:
	IpnPlayerAfact(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerAfact() {}
	virtual void Print() const { printf("IpnPlayerAfact\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerAfact(num), GetLineNum());
	}
};

class IpnPlayerActive: public IpnGameAction {
public:
	IpnPlayerActive(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerActive() {}
	virtual void Print() const { printf("IpnPlayerActive\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerActive(num), GetLineNum());
	}
};

class IpnPlayerDifBought: public IpnGameAction {
public:
	IpnPlayerDifBought(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerDifBought() {}
	virtual void Print() const { printf("IpnPlayerDifBought\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerDifBought(num), GetLineNum());
	}
};

class IpnPlayerDifSold: public IpnGameAction {
public:
	IpnPlayerDifSold(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerDifSold() {}
	virtual void Print() const { printf("IpnPlayerDifSold\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerDifSold(num), GetLineNum());
	}
};

class IpnPlayerLastBought: public IpnGameAction {
public:
	IpnPlayerLastBought(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerLastBought() {}
	virtual void Print() const { printf("IpnPlayerLastBought\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerLastBought(num), GetLineNum());
	}
};

class IpnPlayerLastSold: public IpnGameAction {
public:
	IpnPlayerLastSold(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerLastSold() {}
	virtual void Print() const { printf("IpnPlayerLastSold\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerLastSold(num), GetLineNum());
	}
};

class IpnMinRawPrice: public IpnGameAction {
public:
	IpnMinRawPrice(int l): IpnGameAction(l) {}
	virtual ~IpnMinRawPrice() {}
	virtual void Print() const { printf("IpnMinRawPrice\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMinRawPrice(), GetLineNum());
	}
};

class IpnMaxProdPrice: public IpnGameAction {
public:
	IpnMaxProdPrice(int l): IpnGameAction(l) {}
	virtual ~IpnMaxProdPrice() {}
	virtual void Print() const { printf("IpnMaxProdPrice\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMaxProdPrice(), GetLineNum());
	}
};

class IpnMaxRaw: public IpnGameAction {
public:
	IpnMaxRaw(int l): IpnGameAction(l) {}
	virtual ~IpnMaxRaw() {}
	virtual void Print() const { printf("IpnMaxRaw\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMaxRaw(), GetLineNum());
	}
};

class IpnMaxProd: public IpnGameAction {
public:
	IpnMaxProd(int l): IpnGameAction(l) {}
	virtual ~IpnMaxProd() {}
	virtual void Print() const { printf("IpnMaxProd\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMaxProd(), GetLineNum());
	}
};

class IpnMaxNum: public IpnGameAction {
public:
	IpnMaxNum(int l): IpnGameAction(l) {}
	virtual ~IpnMaxNum() {}
	virtual void Print() const { printf("IpnMaxNum\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMaxNum(), GetLineNum());
	}
};

class IpnMonth: public IpnGameAction {
public:
	IpnMonth(int l): IpnGameAction(l) {}
	virtual ~IpnMonth() {}
	virtual void Print() const { printf("IpnMonth\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMonth(), GetLineNum());
	}
};

class IpnMyRaw: public IpnGameAction {
public:
	IpnMyRaw(int l): IpnGameAction(l) {}
	virtual ~IpnMyRaw() {}
	virtual void Print() const { printf("IpnMyRaw\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMyRaw(), GetLineNum());
	}
};

class IpnMyProd: public IpnGameAction {
public:
	IpnMyProd(int l): IpnGameAction(l) {}
	virtual ~IpnMyProd() {}
	virtual void Print() const { printf("IpnMyProd\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMyProd(), GetLineNum());
	}
};

class IpnMyFact: public IpnGameAction {
public:
	IpnMyFact(int l): IpnGameAction(l) {}
	virtual ~IpnMyFact() {}
	virtual void Print() const { printf("IpnMyFact\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMyFact(), GetLineNum());
	}
};

class IpnMyMoney: public IpnGameAction {
public:
	IpnMyMoney(int l): IpnGameAction(l) {}
	virtual ~IpnMyMoney() {}
	virtual void Print() const { printf("IpnMyMoney\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMyMoney(), GetLineNum());
	}
};

class IpnMyAfact: public IpnGameAction {
public:
	IpnMyAfact(int l): IpnGameAction(l) {}
	virtual ~IpnMyAfact() {}
	virtual void Print() const { printf("IpnMyAfact\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMyAfact(), GetLineNum());
	}
};

class IpnMyNum: public IpnGameAction {
public:
	IpnMyNum(int l): IpnGameAction(l) {}
	virtual ~IpnMyNum() {}
	virtual void Print() const { printf("IpnMyNum\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		return new IpnInt(gInfo->GetMyNum(), GetLineNum());
	}
};

class IpnOpGo: public IpnElem {
public:
	void Print() const { printf("IpnOpGo\n"); }
	IpnOpGo(int l = 0): IpnElem(l) {}
	virtual ~IpnOpGo() {}
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const
	{
		IpnElem *operand = Pop(stack);
		IpnLabel *label = dynamic_cast<IpnLabel*>(operand);
		if (!label)
			throw new IpnExNotLabel(operand->GetLineNum(), InOpGo);
		*CurCmd = label->Get();
		delete operand;
	}
};

class IpnOpGoFalse: public IpnElem {
public:
	void Print() const { printf("IpnOpGoFalse\n"); }
	IpnOpGoFalse(int l = 0): IpnElem(l) {}
	virtual ~IpnOpGoFalse() {}
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const
	{
		double res;
		IpnElem *operand1 = Pop(stack);
		IpnLabel *label = dynamic_cast<IpnLabel*>(operand1);
		if (!label)
			throw new IpnExNotLabel(operand1->GetLineNum(), InOpGoFalse);
		IpnElem *operand2 = Pop(stack);
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand2);
		IpnReal *iReal = dynamic_cast<IpnReal*>(operand2);
		if (iInt) {
			res = iInt->Get();
		} else if (iReal) {
			res = iReal->Get();
		} else {
			delete operand1;
			throw new IpnExNotIntOrReal(operand2->GetLineNum(), InOpGoFalse);
		}
		if (!res) {
			*CurCmd = label->Get();
		} else {
			*CurCmd = (*CurCmd)->next;
		}
		delete operand1;
	}
};

class IpnDie: public IpnElem {
public:
	void Print() const { printf("IpnDie\n"); }
	IpnDie(int l): IpnElem(l) {}
	virtual ~IpnDie() {}
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const
	{
		exit(0);
	}
};

class Ipn {
	IpnItem *FstItem, *LstItem, *BeforeLast;
	IpnBraceL *BraceL;
	IpnBraceR *BraceR;
	IpnVarDesc *VarDesc;
public:
	Ipn(): FstItem(0), LstItem(0), BeforeLast(0), BraceL(0), BraceR(0) {}
	void Add(IpnElem *NewElem);
	void Add(IpnElem *NewElem1, IpnElem *NewElem2);
	void AddNoShift(IpnElem *NewElem);
	void AddNoShift(IpnElem *NewElem1, IpnElem *NewElem2);
	void AddInEnd(IpnElem *NewElem)
		{ LstItem->next->next = new IpnItem(NewElem, LstItem->next->next); }
	IpnItem* Get() { return LstItem; }
	IpnItem* GetLast() { return LstItem->next; }
	IpnItem* GetLastLast() { return LstItem->next->next; }
	void DeleteLast();
	void Shift();
	void Shift(int n);
	void ClearVarDesc();
	int Perform(GameInfo *gInfo);
	void Print(IpnElem *elem);
	~Ipn()
	{
		IpnItem *help;
		while(FstItem != 0) {
			help = FstItem;
			FstItem = FstItem->next;
			delete help->elem;
			delete help;
		}
	}
};

void Ipn::Add(IpnElem *NewElem)
{
	if (FstItem == 0) {
		FstItem = new IpnItem(NewElem, 0);
		LstItem = FstItem;
		BeforeLast = FstItem;
	} else {
		BeforeLast = LstItem;
		LstItem->next = new IpnItem(NewElem, LstItem->next);
		LstItem = LstItem->next;
	}
}

void Ipn::Add(IpnElem *NewElem1, IpnElem *NewElem2)
{
	Add(NewElem1);
	Add(NewElem2);
}

void Ipn::AddNoShift(IpnElem *NewElem)
{
	LstItem->next = new IpnItem(NewElem, LstItem->next);
}

void Ipn::AddNoShift(IpnElem *NewElem1, IpnElem *NewElem2)
{
	AddNoShift(NewElem2);
	AddNoShift(NewElem1);
}

void Ipn::DeleteLast()
{
	IpnItem *help = LstItem;
	BeforeLast->next = LstItem->next;
	delete help->elem;
	delete help;
	LstItem = BeforeLast;
	BeforeLast = FstItem;
	while(BeforeLast->next != LstItem)
		BeforeLast = BeforeLast->next;
}	

void Ipn::Shift()
{
	BeforeLast = LstItem;
	LstItem = LstItem->next;
}

void Ipn::Shift(int n)
{
	for(int i = 0; i < n; i++)
		Shift();
}

void Ipn::ClearVarDesc()
{
	IpnItem *CurCmd = FstItem;
	while(CurCmd != 0) {
		VarDesc = dynamic_cast<IpnVarDesc*>(CurCmd->elem);
		if (VarDesc) {
			VarDesc->Clear();
		}
		CurCmd = CurCmd->next;
	}
}

int Ipn::Perform(GameInfo *gInfo)
{
	IpnItem *stack = 0, *CurCmd = FstItem;
	VarElem *VarList = 0;
	int NestingLevel = 0;
	try
	{
		ClearVarDesc();
		while(CurCmd != 0) {
#ifdef DEBUGGINGINTER
			CurCmd->elem->Print();
#endif
			BraceL = dynamic_cast<IpnBraceL*>(CurCmd->elem);
			BraceR = dynamic_cast<IpnBraceR*>(CurCmd->elem);
			VarDesc = dynamic_cast<IpnVarDesc*>(CurCmd->elem);
			if (BraceL) {
				NestingLevel++;
#ifdef DEBUGGINGINTER
				printf("!%d\n", NestingLevel);
#endif
			}
			if (BraceR) {
				BraceR->DelVar(&VarList, NestingLevel);
#ifdef DEBUGGINGINTER
				printf("!%d\n", NestingLevel);
#endif
			}
			if (VarDesc) {
				VarDesc->AddVar(&stack, &VarList, NestingLevel);
			}
			CurCmd->elem->Evaluate(&stack, &CurCmd, &VarList, gInfo);
		}
	}
	catch(IpnEx *ex)
	{
		ex->HandleError();
		delete ex;
		return 1;
	}
	return 0;
}

class ParsExc {
public:
	ParsExc() {}
	virtual ~ParsExc() {}
	virtual void PrintError() const = 0;
};

class ParsExcLParen: public ParsExc {
	char *str;
public:
	ParsExcLParen(const char *s) { str = strdup(s); }
	virtual ~ParsExcLParen() { delete [] str; }
	virtual void PrintError() const
		{ printf("\"(\" expected after \"%s\" ", str); }
};

class ParsExcComma: public ParsExc {
	char *str;
public:
	ParsExcComma(const char *s) { str = strdup(s); }
	virtual ~ParsExcComma() { delete [] str; }
	virtual void PrintError() const
		{ printf("\",\" expected after the first argument of \"%s\" ", str); }
};

class ParsExcRParen: public ParsExc {
	char *str;
public:
	ParsExcRParen(const char *s) { str = strdup(s); }
	virtual ~ParsExcRParen() { delete [] str; }
	virtual void PrintError() const
		{ printf("\")\" expected after the arguments of \"%s\" ", str); }
};

class BadLex {
	int ErrNum;
public:
	BadLex(int e): ErrNum(e) {}
	int GetErrNum() const { return ErrNum; }
};

struct IntItem {
	int value;
	IntItem *next;
	IntItem(int v, IntItem *n): value(v), next(n) {}
};

class IntStack {
	IntItem *first;
public:
	IntStack(int Init) { first = new IntItem(Init, 0); }
	void Add(int NewElem) { first = new IntItem(NewElem, first); }
	void AddAssign(Ipn *ipn, int LineNum);
	void AddOr(Ipn *ipn, int LineNum);
	void AddAnd(Ipn *ipn, int LineNum);
	void AddComp(Ipn *ipn, int oper, int LineNum);
	void AddAddSub(Ipn *ipn, int oper, int LineNum);
	void AddMulDiv(Ipn *ipn, int oper, int LineNum);
	void AddNeg(Ipn *ipn, int LineNum);
	void MetRParen(Ipn *ipn, int LineNum);
	void AddIpn(Ipn *ipn, int AddValue, int LineNum);
	int Get();
	~IntStack();
};

void IntStack::AddAssign(Ipn *ipn, int LineNum)
{
	int v = first->value;
	if (v==LexLParen || v==LexAssign) {
		ipn->DeleteLast();
		Add(LexAssign);
	} else {
		throw BadLex(LvalueReq);
	}
};

void IntStack::AddOr(Ipn *ipn, int LineNum)
{
	int v = first->value;
	if (v==LexLParen || v==LexAssign) {
		Add(LexOr);
	} else {
		AddIpn(ipn, Get(), LineNum);
		AddOr(ipn, LineNum);
	}
};

void IntStack::AddAnd(Ipn *ipn, int LineNum)
{
	int v = first->value;
	if (v==LexLParen || v==LexOr || v==LexAssign) {
		Add(LexAnd);
	} else {
		AddIpn(ipn, Get(), LineNum);
		AddAnd(ipn, LineNum);
	}
};

void IntStack::AddComp(Ipn *ipn, int oper, int LineNum)
{
	int v = first->value;
	if (v==LexLParen || v==LexOr || v==LexAnd || v==LexAssign) {
		Add(oper);
	} else {
		AddIpn(ipn, Get(), LineNum);
		AddComp(ipn, oper, LineNum);
	}
}

void IntStack::AddAddSub(Ipn *ipn, int oper, int LineNum)
{
	int v = first->value;
	if (v==LexAdd || v==LexSub || v==LexMul || v==LexDiv || v==LexNeg) {
		AddIpn(ipn, Get(), LineNum);
		AddAddSub(ipn, oper, LineNum);
	} else {
		Add(oper);
	}
}

void IntStack::AddMulDiv(Ipn *ipn, int oper, int LineNum)
{
	int v = first->value;
	if (v==LexMul || v==LexDiv || v==LexNeg) {
		AddIpn(ipn, Get(), LineNum);
		AddMulDiv(ipn, oper, LineNum);
	} else {
		Add(oper);
	}
}

void IntStack::AddNeg(Ipn *ipn, int LineNum)
{
	if (first->value == LexNeg) {
		AddIpn(ipn, Get(), LineNum);
		AddNeg(ipn, LineNum);
	} else {
		Add(LexNeg);
	}
}

void IntStack::MetRParen(Ipn *ipn, int LineNum)
{
	int oper = Get();
	while(oper != LexLParen && oper != 0) {
		AddIpn(ipn, oper, LineNum);
		oper = Get();
	}
}

void IntStack::AddIpn(Ipn *ipn, int AddValue, int LineNum)
{
	if (AddValue == LexAssign) {
		ipn->Add(new IpnAssign(LineNum));
	} else if (AddValue == LexOr) {
		ipn->Add(new IpnOr(LineNum));
	} else if (AddValue == LexAnd) {
		ipn->Add(new IpnAnd(LineNum));
	} else if (AddValue == LexLT) {
		ipn->Add(new IpnLess(LineNum));
	} else if (AddValue == LexLE) {
		ipn->Add(new IpnLessE(LineNum));
	} else if (AddValue == LexGT) {
		ipn->Add(new IpnGreat(LineNum));
	} else if (AddValue == LexGE) {
		ipn->Add(new IpnGreatE(LineNum));
	} else if (AddValue == LexEq) {
		ipn->Add(new IpnEqual(LineNum));
	} else if (AddValue == LexNotEq) {
		ipn->Add(new IpnNotEqual(LineNum));
	} else if (AddValue == LexNeg) {
		ipn->Add(new IpnNeg(LineNum));
	} else if (AddValue == LexAdd) {
		ipn->Add(new IpnAdd(LineNum));
	} else if (AddValue == LexSub) {
		ipn->Add(new IpnSub(LineNum));
	} else if (AddValue == LexMul) {
		ipn->Add(new IpnMul(LineNum));
	} else if (AddValue == LexDiv) {
		ipn->Add(new IpnDiv(LineNum));
	}
}

int IntStack::Get()
{
	if (first) {
		int value = first->value;
		IntItem *help = first;
		first = first->next;
		delete help;
		return value;
	}
	return 0;
}

IntStack::~IntStack()
{
	while(first != 0) {
		IntItem *help = first;
		first = first->next;
		delete help;
	}
}

class Parser {
	Automat automat;
	Ipn *ipn;
	Lexeme *current, *last;
	int LexNum, LineNum;
	FILE *f;
	void next();
	void S();
	void Action();
	void IntDesc();
	void RealDesc();
	void StringDesc();
	void WhileDesc();
	void PutDesc();
	void IfElseDesc();
	void ProdDesc();
	void SellDesc();
	void BuyDesc();
	void PrintMyInfoDesc();
	void PrintMarketInfoDesc();
	void PrintPlayerInfoDesc();
	void PlayerProdDesc();
	void PlayerRawDesc();
	void PlayerMoneyDesc();
	void PlayerFactDesc();
	void PlayerAfactDesc();
	void PlayerActiveDesc();
	void PlayerDifBoughtDesc();
	void PlayerDifSoldDesc();
	void PlayerLastBoughtDesc();
	void PlayerLastSoldDesc();
	void ArrayDesc(int DefVal, int RFst, int RScd, int ValFst, int ValScd);
	void Expression(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpOr(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpAnd(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpComp(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpAddSub(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpMulDiv(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpLast(IntStack *stack, int RParenErr, int ValOrIdentErr);
	int CheckComp(int n);
	int CheckInExp(int ErrNum);
	int CheckExp();
public:
	Parser()
		: automat(), current(0), last(0), LexNum(0), f(0) { ipn = new Ipn; }
	Ipn *analyze(FILE *file);
};

Ipn *Parser::analyze(FILE *file)
{
	f = file;
	try {
		next();
		S();
	}
	catch(const BadLex &err) {
		int ErrNum = err.GetErrNum();
		printf("Error in line %d: ", (*current).GetLineNum());
		if (ErrNum == NothingExpect) {
			printf("%s\n", TableOfErrors[ErrNum]);
			return 0;
		} else if (ErrNum == InvalLex) {
			printf("invalid lexeme \"");
		} else if (CheckInExp(ErrNum)) {
			printf("%s after \"", TableOfErrors[ErrNum]);
			(*last).Print();
			printf("\" before \"");
		} else {
			printf("%s before \"", TableOfErrors[ErrNum]);
		}
		(*current).Print();
		printf("\"\n");
		return 0;
	}
	catch(ParsExc *ex)
	{
		printf("Error in line %d: ", (*current).GetLineNum());
		ex->PrintError();
		printf("before \"");
		(*current).Print();
		printf("\"\n");
		delete ex;
		return 0;
	}
	return ipn;
}

void Parser::next()
{
	char c;
	if (last != NULL)
		delete last;
	last = current;
	current = NULL;
	do {
		c = fgetc(f);
		if ((current = automat.FeedChar(c)) != NULL) {
			if (!((*current).CheckCorrect())) 
				throw BadLex(InvalLex);
		}
	} while(current == NULL);
	LexNum = current->GetLexNum();
	LineNum = current->GetLineNum();
}

void Parser::S()
{
	if (LexNum != LexBirth)
		throw BadLex(BirthExpect);
	ipn->Add(new IpnBraceL(current->GetLineNum()));
	next();
	Action();
	if (LexNum != LexDeath)
		throw BadLex(DeathExpect);
	ipn->Add(new IpnBraceR(current->GetLineNum()));
	next();
	if (LexNum != LexEmpty)
		throw BadLex(NothingExpect);
}

void Parser::Action()
{
	if (LexNum == LexDie) {
		ipn->Add(new IpnDie(LineNum));
		next();
	} else if (LexNum == LexInt) {
		next();
		IntDesc();
	} else if (LexNum == LexReal) {
		next();
		RealDesc();
	} else if (LexNum == LexString) {
		next();
		StringDesc();
	} else if (LexNum == LexWhile) {
		next();
		WhileDesc();
	} else if (LexNum == LexPut) {
		next();
		PutDesc();
	} else if (LexNum == LexProd) {
		ProdDesc();
	} else if (LexNum == LexSell) {
		SellDesc();
	} else if (LexNum == LexBuy) {
		BuyDesc();
	} else if (LexNum == LexBuild) {
		ipn->Add(new IpnBuild(LineNum));
		next();
	} else if (LexNum == LexAbuild) {
		ipn->Add(new IpnAbuild(LineNum));
		next();
	} else if (LexNum == LexUpgrade) {
		ipn->Add(new IpnUpgrade(LineNum));
		next();
	} else if (LexNum == LexPrintPlayerInfo) {
		PrintPlayerInfoDesc();
	} else if (LexNum == LexPrintMyInfo) {
		PrintMyInfoDesc();
	} else if (LexNum == LexPrintMarketInfo) {
		PrintMarketInfoDesc();
	} else if (CheckExp()) {
		Expression(0, RParenExp, ValOrIdentExp);
		if (LexNum != LexSemicolon)
			throw BadLex(SemicolonExpect);
		ipn->Add(new IpnSemicolon(LineNum));
		next();
	} else if (LexNum == LexIf) {
		next();
		IfElseDesc();
	} else {
		return;
	}
	Action();
}

int Parser::CheckExp()
{
	return LexNum == LexIdent || LexNum == LexValInt || LexNum == LexValReal
		|| LexNum == LexStr;
}

int Parser::CheckInExp(int ErrNum)
{
	return (
		ErrNum == RParenWhileExp || ErrNum == ValOrIdentWhileExp ||
		ErrNum == RParenPutExp || ErrNum == ValOrIdentPutExp ||
		ErrNum == RParenAssignExp || ErrNum == ValOrIdentAssignExp ||
		ErrNum == RParenIfExp || ErrNum == ValOrIdentIfExp
	);
}

void Parser::ArrayDesc(int DefVal, int RFst, int RScd, int ValFst, int ValScd)
{
	if (LexNum == LexLSB) {
		next();
		Expression(0, RFst, ValFst);
		if (LexNum == LexComma) {
			next();
			Expression(0, RScd, ValScd);
		} else {
			ipn->Add(new IpnInt(DefVal, current->GetLineNum()));
		}
		if (LexNum != LexRSB)
			throw BadLex(RSBExpect);
		next();
	} else {
		ipn->Add(new IpnInt(DefVal, current->GetLineNum()));
		ipn->Add(new IpnInt(DefVal, current->GetLineNum()));
	}
}
		
void Parser::IntDesc()
{
	if (LexNum != LexIdent)
		throw BadLex(IdentAfterInt);  
	char *name = strdup(current->GetName());
	int line = current->GetLineNum();
	next();
	ArrayDesc(1, RpArrFstInt, ValArrFstInt, RpArrScdInt, ValArrScdInt);
	if (LexNum != LexLParen)
		throw BadLex(LParenInt);
	next();
	Expression(0, RParenInitValIntExp, ValOrIdentInitValIntExp);
	if (LexNum != LexRParen)
		throw BadLex(RParenInt);
	next();
	ipn->Add(new IpnVarDesc(name, Int, line));
	delete [] name;
	if (LexNum == LexComma) {
		next();
		IntDesc();
	}
}

void Parser::RealDesc()
{
	if (LexNum != LexIdent)
		throw BadLex(IdentAfterReal);
	char *name = strdup(current->GetName());
	int line = current->GetLineNum();
	next();
	ArrayDesc(1, RpArrFstReal, ValArrFstReal, RpArrScdReal, ValArrScdReal);
	if (LexNum != LexLParen)
		throw BadLex(LParenReal);
	next();
	Expression(0, RParenInitValRealExp, ValOrIdentInitValRealExp);
	if (LexNum != LexRParen)
		throw BadLex(RParenReal);
	next();
	ipn->Add(new IpnVarDesc(name, Real, line));
	delete [] name;
	if (LexNum == LexComma) {
		next();
		RealDesc();
	}
}

void Parser::StringDesc()
{
	if (LexNum != LexIdent)
		throw BadLex(IdentAfterStr);
	char *name = strdup(current->GetName());
	int line = current->GetLineNum();
	next();
	ipn->Add(new IpnInt(1, line), new IpnInt(1, line));
	if (LexNum != LexLParen)
		throw BadLex(LParenStr);
	next();
	if (LexNum != LexStr)
		throw BadLex(StrExpect);
	ipn->Add(new IpnString(current->GetString(), LineNum));
	next();
	if (LexNum != LexRParen)
		throw BadLex(RParenStr);
	next();
	ipn->Add(new IpnVarDesc(name, String, line));
	delete [] name;
	if (LexNum == LexComma) {
		next();
		StringDesc();
	}
}

void Parser::WhileDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenWhile);
	ipn->Add(new IpnBraceL(LineNum));
	next();
	ipn->Add(new IpnNoOp(LineNum));
	IpnLabel *label = new IpnLabel(ipn->Get(), LineNum);
	Expression(0, RParenWhileExp, ValOrIdentWhileExp);
	ipn->AddNoShift(new IpnNoOp(LineNum));
	ipn->Add(new IpnLabel(ipn->GetLast(), LineNum));
	ipn->Add(new IpnOpGoFalse(LineNum));
	ipn->AddNoShift(label, new IpnOpGo(LineNum));
	if (LexNum != LexRParen)
		throw BadLex(RParenWhile);
	next();
	if (LexNum != LexLB)
		throw BadLex(LBWhile);
	next();
	Action();
	ipn->Shift(3);
	if (LexNum != LexRB)
		throw BadLex(RBWhile);
	ipn->Add(new IpnBraceR(LineNum));
	next();
}

void Parser::PutDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenPut);
	ipn->Add(new IpnEndOfArg(LineNum));
	next();
	Expression(0, RParenPutExp, ValOrIdentPutExp);
	while(LexNum == LexComma) {
		next();
		Expression(0, RParenPutExp, ValOrIdentPutExp);
	}
	if (LexNum != LexRParen)
		throw BadLex(RParenPut);
	ipn->Add(new IpnPut(LineNum));
	next();
}

void Parser::IfElseDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenIf);
	ipn->Add(new IpnBraceL(LineNum));
	next();
	Expression(0, RParenIfExp, ValOrIdentIfExp);
	ipn->AddNoShift(new IpnNoOp(LineNum));
	ipn->Add(new IpnLabel(ipn->GetLast(), LineNum));
	ipn->Add(new IpnOpGoFalse(LineNum));
	if (LexNum != LexRParen)
		throw BadLex(RParenIf);
	next();
	if (LexNum != LexLB)
		throw BadLex(LBIfExpect);
	next();
	Action();
	if (LexNum != LexRB)
		throw BadLex(RBIfExpect);
	next();
	if (LexNum == LexElse) {
		next();
		ipn->AddInEnd(new IpnNoOp(LineNum));
		ipn->Add(new IpnLabel(ipn->GetLastLast(), LineNum));
		ipn->Add(new IpnOpGo(LineNum));
		ipn->Shift();
		if (LexNum != LexLB)
			throw BadLex(LBElseExpect);
		next();
		Action();
		if (LexNum != LexRB)
			throw BadLex(RBElseExpect);
		next();
	}
	ipn->Shift();
	ipn->Add(new IpnBraceR(last->GetLineNum()));
}

void Parser::ProdDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("prod");
	next();
	Expression(0, RParenProdExp, ValOrIdentProdExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("prod");
	next();
	ipn->Add(new IpnProd(line));
}

void Parser::SellDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("sell");
	next();
	Expression(0, RParenSellFstExp, ValOrIdentSellFstExp);
	if (LexNum != LexComma)
		throw new ParsExcComma("sell");
	next();
	Expression(0, RParenSellScdExp, ValOrIdentSellScdExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("sell");
	next();
	ipn->Add(new IpnSell(line));
}

void Parser::BuyDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("buy");
	next();
	Expression(0, RParenBuyFstExp, ValOrIdentBuyFstExp);
	if (LexNum != LexComma)
		throw new ParsExcComma("buy");
	next();
	Expression(0, RParenBuyScdExp, ValOrIdentBuyScdExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("buy");
	next();
	ipn->Add(new IpnBuy(line));
}

void Parser::PrintPlayerInfoDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PrintPlayerInfo");
	next();
	Expression(0, RParenPrintPlayerInfoExp, ValOrIdentPrintPlayerInfoExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PrintPlayerInfo");
	next();
	ipn->Add(new IpnPrintPlayerInfo(line));
}

void Parser::PrintMyInfoDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PrintMyInfo");
	next();
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PrintMyInfo");
	next();
	ipn->Add(new IpnPrintMyInfo(line));
}

void Parser::PrintMarketInfoDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PrintMarketInfo");
	next();
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PrintMarketInfo");
	next();
	ipn->Add(new IpnPrintMarketInfo(line));
}

void Parser::Expression(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	int Declared = 0;
	if (stack == 0) {
		stack = new IntStack(LexLParen);
		Declared = 1;
	}
	ExpOr(stack, RParenErr, ValOrIdentErr);
	while(LexNum == LexAssign) {
		stack->AddAssign(ipn, LineNum);
		next();
		ExpOr(stack, RParenErr, ValOrIdentErr);
	}
	if (Declared) {
		stack->MetRParen(ipn, LineNum);
		delete stack;
	}
}

void Parser::ExpOr(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	ExpAnd(stack, RParenErr, ValOrIdentErr);
	while(LexNum == LexOr) {
		stack->AddOr(ipn, LineNum);
		next();
		ExpAnd(stack, RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpAnd(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	ExpComp(stack, RParenErr, ValOrIdentErr);
	while(LexNum == LexAnd) {
		stack->AddAnd(ipn, LineNum);
		next();
		ExpComp(stack, RParenErr, ValOrIdentErr);
	}
}

int Parser::CheckComp(int n)
{
	return n==LexLT||n==LexLE||n==LexGT||n==LexGE||n==LexEq||n==LexNotEq;
}

void Parser::ExpComp(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	ExpAddSub(stack, RParenErr, ValOrIdentErr);
	while(CheckComp(LexNum)) {
		stack->AddComp(ipn, LexNum, LineNum);
		next();
		ExpAddSub(stack, RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpAddSub(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	if (LexNum == LexAdd || LexNum == LexSub) {
		ipn->Add(new IpnInt(0, LineNum));
		stack->AddAddSub(ipn, LexNum, LineNum);
		next();
	}
	ExpMulDiv(stack, RParenErr, ValOrIdentErr);
	while(LexNum == LexAdd || LexNum == LexSub) {
		stack->AddAddSub(ipn, LexNum, LineNum);
		next();
		ExpMulDiv(stack, RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpMulDiv(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	ExpLast(stack, RParenErr, ValOrIdentErr);
	while(LexNum == LexMul || LexNum == LexDiv) {
		stack->AddMulDiv(ipn, LexNum, LineNum);
		next();
		ExpLast(stack, RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpLast(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	if (LexNum == LexIdent) {
		char *name = strdup(current->GetName());
		int line = current->GetLineNum();
		next();
		ArrayDesc(0, RpArrFstExp, ValArrFstExp, RpArrScdExp, ValArrScdExp);
		ipn->Add(new IpnVarAddr(name, line));
		ipn->Add(new IpnTakeValue(line));
		delete [] name;
	} else if (LexNum == LexValInt) {
		ipn->Add(new IpnInt(current->GetInt(), LineNum));
		next();
	} else if (LexNum == LexValReal) {
		ipn->Add(new IpnReal(current->GetReal(), LineNum));
		next();
	} else if (LexNum == LexStr) {
		ipn->Add(new IpnString(current->GetString(), LineNum));
		next();
	} else if (LexNum == LexNeg) {
		stack->AddNeg(ipn, LineNum);
		next();
		ExpLast(stack, RParenErr, ValOrIdentErr);
	} else if (LexNum == LexLParen) {
		stack->Add(LexLParen);
		next();
		Expression(stack, RParenErr, ValOrIdentErr);
		if (LexNum != LexRParen)
			throw BadLex(RParenErr);
		stack->MetRParen(ipn, LineNum);
		next();
	} else if (LexNum == LexMinRawPrice) {
		ipn->Add(new IpnMinRawPrice(LineNum));
		next();
	} else if (LexNum == LexMaxProdPrice) {
		ipn->Add(new IpnMaxProdPrice(LineNum));
		next();
	} else if (LexNum == LexMaxRaw) {
		ipn->Add(new IpnMaxRaw(LineNum));
		next();
	} else if (LexNum == LexMaxProd) {
		ipn->Add(new IpnMaxProd(LineNum));
		next();
	} else if (LexNum == LexMaxNum) {
		ipn->Add(new IpnMaxNum(LineNum));
		next();
	} else if (LexNum == LexMonth) {
		ipn->Add(new IpnMonth(LineNum));
		next();
	} else if (LexNum == LexMyProd) {
		ipn->Add(new IpnMyProd(LineNum));
		next();
	} else if (LexNum == LexMyRaw) {
		ipn->Add(new IpnMyRaw(LineNum));
		next();
	} else if (LexNum == LexMyFact) {
		ipn->Add(new IpnMyFact(LineNum));
		next();
	} else if (LexNum == LexMyMoney) {
		ipn->Add(new IpnMyMoney(LineNum));
		next();
	} else if (LexNum == LexMyAfact) {
		ipn->Add(new IpnMyAfact(LineNum));
		next();
	} else if (LexNum == LexMyNum) {
		ipn->Add(new IpnMyNum(LineNum));
		next();
	} else if (LexNum == LexPlayerProd) {
		PlayerProdDesc();
	} else if (LexNum == LexPlayerRaw) {
		PlayerRawDesc();
	} else if (LexNum == LexPlayerFact) {
		PlayerFactDesc();
	} else if (LexNum == LexPlayerMoney) {
		PlayerMoneyDesc();
	} else if (LexNum == LexPlayerAfact) {
		PlayerAfactDesc();
	} else if (LexNum == LexPlayerActive) {
		PlayerActiveDesc();
	} else if (LexNum == LexPlayerDifBought) {
		PlayerDifBoughtDesc();
	} else if (LexNum == LexPlayerDifSold) {
		PlayerDifSoldDesc();
	} else if (LexNum == LexPlayerLastBought) {
		PlayerLastBoughtDesc();
	} else if (LexNum == LexPlayerLastSold) {
		PlayerLastSoldDesc();
	} else {
		throw BadLex(ValOrIdentErr);
	}
}

void Parser::PlayerProdDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerProd");
	next();
	Expression(0, RParenPlayerProdExp, ValOrIdentPlayerProdExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerProd");
	next();
	ipn->Add(new IpnPlayerProd(line));
}

void Parser::PlayerRawDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerRaw");
	next();
	Expression(0, RParenPlayerRawExp, ValOrIdentPlayerRawExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerRaw");
	next();
	ipn->Add(new IpnPlayerRaw(line));
}

void Parser::PlayerFactDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerFact");
	next();
	Expression(0, RParenPlayerFactExp, ValOrIdentPlayerFactExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerFact");
	next();
	ipn->Add(new IpnPlayerFact(line));
}

void Parser::PlayerMoneyDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerMoney");
	next();
	Expression(0, RParenPlayerMoneyExp, ValOrIdentPlayerMoneyExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerMoney");
	next();
	ipn->Add(new IpnPlayerMoney(line));
}

void Parser::PlayerAfactDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerAfact");
	next();
	Expression(0, RParenPlayerAfactExp, ValOrIdentPlayerAfactExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerAfact");
	next();
	ipn->Add(new IpnPlayerAfact(line));
}

void Parser::PlayerActiveDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerActive");
	next();
	Expression(0, RParenPlayerActiveExp, ValOrIdentPlayerActiveExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerActive");
	next();
	ipn->Add(new IpnPlayerActive(line));
}

void Parser::PlayerDifBoughtDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerDifBought");
	next();
	Expression(0, RParenDifBoughtExp, ValOrIdentDifBoughtExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerDifBought");
	next();
	ipn->Add(new IpnPlayerDifBought(line));
}

void Parser::PlayerDifSoldDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerDifSold");
	next();
	Expression(0, RParenDifSoldExp, ValOrIdentDifSoldExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerDifSold");
	next();
	ipn->Add(new IpnPlayerDifSold(line));
}

void Parser::PlayerLastBoughtDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerLastBought");
	next();
	Expression(0, RParenLastBoughtExp, ValOrIdentLastBoughtExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerLastBought");
	next();
	ipn->Add(new IpnPlayerLastBought(line));
}

void Parser::PlayerLastSoldDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerLastSold");
	next();
	Expression(0, RParenLastSoldExp, ValOrIdentLastSoldExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerLastSold");
	next();
	ipn->Add(new IpnPlayerLastSold(line));
}

int main(int argc, char **argv)
{
	Parser parser;
	Ipn *ipn;
	FILE *f = fopen(argv[Script], "r");
	if (!f)
		return 0;
	if (!(ipn = parser.analyze(f)))
		return 0;
	try 
	{
		GameInfo gInfo(argc, argv);
		for(;;) {
			gInfo.UpdateInfo();	
			if (ipn->Perform(&gInfo))
				return 0;
			gInfo.WaitAuction();
		}
	}
	catch(const char *s) 
	{
		printf("%s\n", s);
	}
	delete ipn;
	return 0;
}
