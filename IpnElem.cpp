#include "IpnElem.h"

IpnElem *IpnElem::Pop(IpnItem **stack) const
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

int IpnVarOrAssign::GetType(VarElem **VarList, char *search) const
{
	VarElem *cur = *VarList;
	while(cur != 0) {
		if (!strcmp(cur->name, search))
			return cur->type;
		cur = cur->next;
	}
	return NotFound;
}

void IpnVarOrAssign::GetDims(
	IpnItem **stack, int &FstDim, int &ScdDim, int inDesc
) const
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

void IpnVarOrAssign::Evaluate(
	IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
) const
{
	IpnElem *res = EvaluateFun(stack, VarList);
	if (res)
		Push(stack, res);
	*CurCmd = (*CurCmd)->next;
}

char *IpnTakeValue::GetString(VarElem **VarList, char *name) const
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

IpnElem *IpnTakeValue::EvaluateFun(IpnItem **stack, VarElem **VarList) const
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

void IpnAssign::SetString(VarElem **VarList, IpnVarAddr *VarAddr,
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

IpnElem *IpnAssign::EvaluateFun(IpnItem **stack, VarElem **VarList) const
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
			long long res = iInt->Get();
			SetValue<double, IpnInt *>(stack, VarList, iVarAddr, iInt);
			return new IpnInt(res, GetLineNum());
		} else if (iReal) {
			double res = iReal->Get();
			SetValue<double, IpnReal *>(stack, VarList, iVarAddr, iReal);
			return new IpnReal(res, GetLineNum());
		} else {
			throw new IpnExNotIntOrReal(operand2->GetLineNum(), InAssign);
		}
	} else if (type == Int) {
		if (iInt) {
			long long res = iInt->Get();
			SetValue<long long, IpnInt *>(stack, VarList, iVarAddr, iInt);
			return new IpnInt(res, GetLineNum());
		} else {
			throw new IpnExNotInt(operand2->GetLineNum(), InAssign);
		}
	} else if (type == String) {
		if (iString) {
			char *s = strdup(iString->Get());
			SetString(VarList, iVarAddr, iString);
			return new IpnString(s, GetLineNum());
		} else {
			throw new IpnExNotStr(operand2->GetLineNum(), InAssign);
		}
	}
	return 0;
}

int IpnVarDesc::SearchName(VarElem *VarList)
{
	while(VarList != 0) {
		if (!strcmp(VarList->name, name))
			return 1;
		VarList = VarList->next;
	}
	return 0;
}

void IpnVarDesc::AddInVarList(VarElem **VarList, int level)
{
	VarElem *NewElem;
	NewElem = new VarElem(name, type, value, FstDim, ScdDim, level);
	NewElem->next = *VarList;
	*VarList = NewElem;
	considered = 1;
}

IpnVarDesc::~IpnVarDesc()
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

void IpnVarDesc::AddVar(IpnItem **stack, VarElem **VarList, int level)
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

void IpnFunction::Evaluate(
	IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
) const
{
	IpnElem *res = EvaluateFun(stack);
	if (res)
		Push(stack, res);
	*CurCmd = (*CurCmd)->next;
}

IpnElem *IpnArithmeticOperation::EvaluateFun(IpnItem **stack) const
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

IpnElem *IpnLogicalOperation::EvaluateFun(IpnItem **stack) const
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

IpnElem *IpnNeg::EvaluateFun(IpnItem **stack) const
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

IpnElem *IpnPut::EvaluateFun(IpnItem **stack) const
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

IpnElem *IpnSemicolon::EvaluateFun(IpnItem **stack) const
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

void IpnBraceR::DelVar(VarElem **VarList, int &NestingLevel)
{
	VarElem *help;
	while(*VarList != 0 && (*VarList)->level == NestingLevel) {
		help = *VarList;
		*VarList = (*VarList)->next;
		delete help;
	}
	NestingLevel--;
}

long long IpnGameAction::GetInt(IpnItem **stack) const
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

long long IpnGameAction::GetIntCheck(IpnItem **stack, GameInfo *gInfo) const
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

void IpnGameAction::Evaluate(
	IpnItem **stack, IpnItem **CurCmd, VarElem **VarList, GameInfo *gInfo
) const
{
	IpnElem *res = MakeAction(stack, gInfo);
	if (res)
		Push(stack, res);
	*CurCmd = (*CurCmd)->next;
}

void IpnOpGo::Evaluate(
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

void IpnOpGoFalse::Evaluate(
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

