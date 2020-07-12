#ifndef IPNEX_H
#define IPNEX_H

#include <stdio.h>
#include "MyString.h"

enum {InFstDim, InScdDim, InVarDesc, InAssign, InAddFst, InAddScd,
	InSubFst, InSubScd, InMulFst, InMulScd, InDivFst, InDivScd,
	InOrFst, InOrScd, InAndFst, InAndScd, InLessFst, InLessScd,
	InLessEFst, InLessEScd, InGreatFst, InGreatScd,
	InGreatEFst, InGreatEScd, InEqualFst, InEqualScd,
	InNotEqualFst, InNotEqualScd, InNeg, InUnSub, InOpGoFalse, InPut,
	InSemicolon, InVar, InOpGo, InTakeValue
};

class IpnEx {
	int LineNum;
public:
	IpnEx(int l = 0): LineNum(l) {}
	virtual ~IpnEx() {}
	int GetLineNum() { return LineNum; }
	virtual void PrintError() const = 0;
	void HandleError() const;
};	

class IpnExNotInt: public IpnEx {
	int where;
public:
	IpnExNotInt(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotInt() {}
	virtual void PrintError() const;
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
	virtual void PrintError() const;
};

class IpnExNotStr: public IpnEx {
	int where;
public:
	IpnExNotStr(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotStr() {}
	virtual void PrintError() const;
};

class IpnExNotIntOrRealOrString: public IpnEx {
	int where;
public:
	IpnExNotIntOrRealOrString(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotIntOrRealOrString() {}
	virtual void PrintError() const;
};
	
class IpnExNotFound: public IpnEx {
	int where;
	char *name;
public:
	IpnExNotFound(int l, int w, char *n): IpnEx(l), where(w)
		{ name = MyStrdup(n); }
	virtual ~IpnExNotFound() { delete [] name; }
	virtual void PrintError() const;
};

class IpnExRedec: public IpnEx {
	int where;
	char *name;
public:
	IpnExRedec(int l, int w, char *n): IpnEx(l), where(w)
		{ name = MyStrdup(n); }
	virtual ~IpnExRedec() { delete [] name; }
	virtual void PrintError() const;
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
	IpnExSegFault(int l, char *n): IpnEx(l) { name = MyStrdup(n); }
	virtual ~IpnExSegFault() { if (name) delete [] name; }
	virtual void PrintError() const;
};

class IpnExNotLabel: public IpnEx {
	int where;
public:
	IpnExNotLabel(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotLabel() {}
	virtual void PrintError() const;
};

class IpnExNotVarAddr: public IpnEx {
	int where;
public:
	IpnExNotVarAddr(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotVarAddr() {}
	virtual void PrintError() const;
};
#endif
