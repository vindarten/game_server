#include "StackOperations.h"

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
	if (v==LexAdd||v==LexSub||v==LexMul||v==LexDiv||v==LexNeg||v==LexUnSub) {
		AddIpn(ipn, Get(), LineNum);
		AddAddSub(ipn, oper, LineNum);
	} else {
		Add(oper);
	}
}

void IntStack::AddMulDiv(Ipn *ipn, int oper, int LineNum)
{
	int v = first->value;
	if (v==LexMul || v==LexDiv || v==LexNeg || v==LexUnSub) {
		AddIpn(ipn, Get(), LineNum);
		AddMulDiv(ipn, oper, LineNum);
	} else {
		Add(oper);
	}
}

void IntStack::AddUnSub(Ipn *ipn, int LineNum)
{
	if (first->value == LexNeg || first->value == LexUnSub) {
		AddIpn(ipn, Get(), LineNum);
		AddUnSub(ipn, LineNum);
	} else {
		Add(LexUnSub);
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
	} else if (AddValue == LexUnSub) {
		ipn->Add(new IpnUnSub(LineNum));
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

