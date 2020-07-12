#ifndef IPNELEM_H
#define IPNELEM_H

#include <stdio.h>
#include <stdlib.h>
#include "MyString.h"
#include "IpnEx.h"
#include "GameInfo.h"

enum {NotFound, Int, Real, String};

struct VarElem {
	int type, FstDim, ScdDim, level;
	char *name;
	void *value;
	VarElem *next;
	VarElem(char *n, int t, void *v, int f, int s, int l)
		: type(t), FstDim(f), ScdDim(s), level(l), name(n), value(v) {}
};

class IpnElem;

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
		{ *stack = new IpnItem(elem, *stack); }
	IpnElem *Pop(IpnItem **stack) const;
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
	IpnString(const char *s, int l = 0): IpnConst(l) { str = MyStrdup(s); }
	IpnString(const IpnString &other) { str = MyStrdup(other.str); }
	virtual ~IpnString() { delete [] str; }
	virtual IpnString *Clone() const
		{ return new IpnString(str, GetLineNum()); }
	char *Get() const { return str; }
	void Print() const { printf("IpnString %s\n", str); }
};

class IpnVarAddr: public IpnConst {
	char *str;
public:
	IpnVarAddr(const char *s, int l = 0): IpnConst(l) { str = MyStrdup(s); }
	IpnVarAddr(const IpnVarAddr &other) { str = MyStrdup(other.str); }
	virtual ~IpnVarAddr() { delete [] str; }
	virtual IpnVarAddr *Clone() const
		{ return new IpnVarAddr(str, GetLineNum()); }
	char *Get() const { return str; }
	void Print() const { printf("IpnVarAddr %s\n", str); }
};

class IpnVarOrAssign: public IpnElem {
protected:
	int GetType(VarElem **VarList, char *search) const;
	void GetDims(IpnItem **stack, int &FstDim, int &ScdDim, int inDesc) const;
public:
	IpnVarOrAssign(int l = 0): IpnElem(l) {}
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const = 0;
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const;
};

class IpnTakeValue: public IpnVarOrAssign {
	template <class T>
	T GetValue(VarElem **VarList, int FstDim, int ScdDim, char *name) const
	{
		VarElem *cur = *VarList;
		while(cur != 0) {
			if (!MyStrcmp(cur->name, name)) {
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
	char *GetString(VarElem **VarList, char *name) const;
public:
	IpnTakeValue(int l = 0): IpnVarOrAssign(l) {}
	virtual ~IpnTakeValue() {}
	void Print() const { printf("IpnTakeValue\n"); }
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const;
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
			if (!MyStrcmp(cur->name, name)) {
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
		IpnString *iString) const;
public:
	void Print() const { printf("IpnAssign\n"); }
	IpnAssign(int l = 0): IpnVarOrAssign(l) {}
	virtual ~IpnAssign() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const;
};

class IpnVarDesc: public IpnVarOrAssign {
	int type, FstDim, ScdDim, considered;
	char *name;
	void *value;
	int SearchName(VarElem *VarList);
	void AddInVarList(VarElem **VarList, int level);
public:
	void Print() const { printf("IpnVarDesc %s\n", name); }
	IpnVarDesc(char *n, int t, int l = 0)
		: IpnVarOrAssign(l), type(t), FstDim(0), ScdDim(0), 
		considered(0), value(0)
		{ name = MyStrdup(n);	}
	~IpnVarDesc();
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const
		{ return 0; }
	void Clear() { considered = 0; }
	void AddVar(IpnItem **stack, VarElem **VarList, int level);
};

class IpnFunction: public IpnElem {
public:
	IpnFunction(int l = 0): IpnElem(l) {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const = 0;
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const;
};

class IpnArithmeticOperation: public IpnFunction {
public:
	IpnArithmeticOperation(int l = 0): IpnFunction(l) {}
	virtual ~IpnArithmeticOperation() {}
	virtual long long CalcInt(long long x, long long y) const = 0;
	virtual double CalcReal(double x, double y) const = 0;
	virtual void ThrowFst(int LineNum) const = 0;
	virtual void ThrowScd(int LineNum) const = 0;
	virtual IpnElem *EvaluateFun(IpnItem **stack) const;
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
	virtual IpnElem *EvaluateFun(IpnItem **stack) const;
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
	IpnNotEqual(int l): IpnLogicalOperation(l) {}
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
	IpnNeg(int l): IpnFunction(l) {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const;
};

class IpnUnSub: public IpnFunction {
public:
	void Print() const { printf("IpnUnSub\n"); }
	IpnUnSub(int l): IpnFunction(l) {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const;
};

class IpnPut: public IpnFunction {
public:
	void Print() const { printf("IpnPut\n"); }
	IpnPut(int l = 0): IpnFunction(l) {}
	virtual ~IpnPut() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const;
};

class IpnSemicolon: public IpnFunction {
public:
	void Print() const { printf("IpnSemicolon\n"); }
	IpnSemicolon(int l = 0): IpnFunction(l) {}
	virtual ~IpnSemicolon() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const;
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
	void DelVar(VarElem **VarList, int &NestingLevel);
};

class IpnGameAction: public IpnElem {
protected:
	long long GetInt(IpnItem **stack) const;
	long long GetIntCheck(IpnItem **stack, GameInfo *gInfo) const;
public:
	IpnGameAction(int l = 0): IpnElem(l) {}
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const = 0;
	void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const;
};

class IpnProd: public IpnGameAction {
public:
	IpnProd(int l): IpnGameAction(l) {}
	virtual ~IpnProd() {}
	virtual void Print() const { printf("IpnProd\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		gInfo->SendProd(GetInt(stack));
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
		gInfo->SendSell(GetInt(stack), GetInt(stack));
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
		gInfo->SendBuy(GetInt(stack), GetInt(stack));
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
		gInfo->SendBuild();
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
		gInfo->SendAbuild();
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
		gInfo->SendUpgrade();
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

class IpnPlayerLastDifBought: public IpnGameAction {
public:
	IpnPlayerLastDifBought(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerLastDifBought() {}
	virtual void Print() const { printf("IpnPlayerLastDifBought\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerLastDifBought(num), GetLineNum());
	}
};

class IpnPlayerLastDifSold: public IpnGameAction {
public:
	IpnPlayerLastDifSold(int l): IpnGameAction(l) {}
	virtual ~IpnPlayerLastDifSold() {}
	virtual void Print() const { printf("IpnPlayerLastDifSold\n"); }
	virtual IpnElem *MakeAction(IpnItem **stack, GameInfo *gInfo) const
	{
		long long num = GetIntCheck(stack, gInfo);
		return new IpnInt(gInfo->GetPlayerLastDifSold(num), GetLineNum());
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
	) const;
};

class IpnOpGoFalse: public IpnElem {
public:
	void Print() const { printf("IpnOpGoFalse\n"); }
	IpnOpGoFalse(int l = 0): IpnElem(l) {}
	virtual ~IpnOpGoFalse() {}
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
	) const;
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
#endif
