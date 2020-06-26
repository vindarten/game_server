#include "Ipn.h"

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
	int NestingLevel = 0;
	while(CurCmd != 0) {
		IpnBraceL *BraceL = dynamic_cast<IpnBraceL*>(CurCmd->elem);
		IpnBraceR *BraceR = dynamic_cast<IpnBraceR*>(CurCmd->elem);
		IpnVarDesc *VarDesc = dynamic_cast<IpnVarDesc*>(CurCmd->elem);
		if (BraceL)
			NestingLevel++;
		if (BraceR)
			NestingLevel--;
		if (VarDesc && NestingLevel) {
			VarDesc->Clear();
		}
		CurCmd = CurCmd->next;
	}
}

int Ipn::Perform(GameInfo *gInfo)
{
	IpnItem *stack = 0, *CurCmd = FstItem;
	int NestingLevel = 0;
	try
	{
		ClearVarDesc();
		while(CurCmd != 0) {
#ifdef DEBUGGINGINTER
			CurCmd->elem->Print();
#endif
			IpnBraceL *BraceL = dynamic_cast<IpnBraceL*>(CurCmd->elem);
			IpnBraceR *BraceR = dynamic_cast<IpnBraceR*>(CurCmd->elem);
			IpnVarDesc *VarDesc = dynamic_cast<IpnVarDesc*>(CurCmd->elem);
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

Ipn::~Ipn()
{
	IpnItem *help;
	while(FstItem != 0) {
		help = FstItem;
		FstItem = FstItem->next;
		delete help->elem;
		delete help;
	}
}

