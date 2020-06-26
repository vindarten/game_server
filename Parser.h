#ifndef PARSER_H
#define PARSER_H

#include "Ipn.h"
#include "StackOperations.h"
#include "automat.h"

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
	void PlayerLastDifBoughtDesc();
	void PlayerLastDifSoldDesc();
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

#endif
