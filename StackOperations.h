#ifndef STACKOPERATIONS_H
#define STACKOPERATIONS_H

#include "Ipn.h"
#include "lexeme.h"

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

class BadLex {
	int ErrNum;
public:
	BadLex(int e): ErrNum(e) {}
	int GetErrNum() const { return ErrNum; }
};

#endif
