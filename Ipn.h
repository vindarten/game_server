#ifndef IPN_H
#define IPN_H

#include "IpnElem.h"

class Ipn {
	IpnItem *FstItem, *LstItem, *BeforeLast;
	VarElem *VarList;
	void ClearVarDesc();
public:
	Ipn(): FstItem(0), LstItem(0), BeforeLast(0), VarList(0) {}
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
	int Perform(GameInfo *gInfo);
	void Print(IpnElem *elem);
	~Ipn();
};

#endif
