#include "Parser.h"

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
	int c;
	if (last != NULL)
		delete last;
	last = current;
	current = NULL;
	if (EndSec) {
		current = new Lexeme(0, LexEmpty);
	} else {
		do {
			if ((c = fgetc(f)) == EOF) {
				if ((current = automat.FeedChar('\n')) != NULL) {
					EndSec = 1;
					if (!((*current).CheckCorrect())) 
						throw BadLex(InvalLex);
				} else {
					current = new Lexeme(0, LexEmpty);
				}
			} else {
				if ((current = automat.FeedChar(c)) != NULL) {
					if (!((*current).CheckCorrect())) 
						throw BadLex(InvalLex);
				}
			}
		} while(current == NULL);
	}
	LexNum = current->GetLexNum();
	LineNum = current->GetLineNum();
}

void Parser::S()
{
	while(LexNum == LexInt || LexNum == LexReal || LexNum == LexString) {
		if (LexNum == LexInt) {
			next();
			IntDesc();
		} else if (LexNum == LexReal) {
			next();
			RealDesc();
		} else if (LexNum == LexString) {
			next();
			StringDesc();
		}
	}
	if (LexNum != LexBirth)
		throw BadLex(BirthExpect);
	ipn->Add(new IpnBraceL(LineNum));
	next();
	Action();
	if (LexNum != LexDeath)
		throw BadLex(DeathExpect);
	ipn->Add(new IpnBraceR(LineNum));
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
	char *name = MyStrdup(current->GetName());
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
	char *name = MyStrdup(current->GetName());
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
	char *name = MyStrdup(current->GetName());
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
		char *name = MyStrdup(current->GetName());
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
	} else if (LexNum == LexSub) {
		stack->AddUnSub(ipn, LineNum);
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
	} else if (LexNum == LexPlayerLastDifBought) {
		PlayerLastDifBoughtDesc();
	} else if (LexNum == LexPlayerLastDifSold) {
		PlayerLastDifSoldDesc();
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

void Parser::PlayerLastDifBoughtDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerLastDifBought");
	next();
	Expression(0, RParenLastDifBoughtExp, ValOrIdentLastDifBoughtExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerLastDifBought");
	next();
	ipn->Add(new IpnPlayerLastDifBought(line));
}

void Parser::PlayerLastDifSoldDesc()
{
	int line = current->GetLineNum();
	next();
	if (LexNum != LexLParen)
		throw new ParsExcLParen("PlayerLastDifSold");
	next();
	Expression(0, RParenLastDifSoldExp, ValOrIdentLastDifSoldExp);
	if (LexNum != LexRParen)
		throw new ParsExcRParen("PlayerLastDifSold");
	next();
	ipn->Add(new IpnPlayerLastDifSold(line));
}

