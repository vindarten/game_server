#ifndef TABLES_H
#define TABLES_H

enum {
	LexEmpty, LexIdent, LexValInt, LexValReal, LexStr, LexBirth, LexDeath,
	LexDie, LexInt, LexReal, LexString,
	LexIf, LexElse, LexWhile, LexPut, LexGet, LexComma, LexSemicolon,
	LexAssign,
	LexLB, LexRB, LexLSB, LexRSB, LexLParen, LexRParen,
	LexOr, LexAnd, LexLT, LexLE, LexGT, LexGE, LexEq, LexNotEq, LexAdd, LexSub,
	LexUnSub, LexMul, LexDiv, LexNeg,
	LexProd, LexSell, LexBuy, LexBuild, LexAbuild, LexUpgrade,
	LexMinRawPrice, LexMaxProdPrice, LexMaxRaw, LexMaxProd, LexMaxNum, LexMonth,
	LexMyProd, LexMyRaw, LexMyFact, LexMyMoney, LexMyAfact, LexMyNum,
	LexPlayerProd, LexPlayerRaw, LexPlayerFact, LexPlayerMoney, LexPlayerAfact,
	LexPlayerActive,
	LexPlayerDifBought, LexPlayerDifSold,
	LexPlayerLastDifBought, LexPlayerLastDifSold,
	LexPlayerLastBought, LexPlayerLastSold,
	LexPrintMyInfo, LexPrintMarketInfo, LexPrintPlayerInfo,
	LexError
};

enum {
	InvalLex, BirthExpect, DeathExpect, NothingExpect, SemicolonExpect,
	IdentAfterInt,
	RpArrFstInt, ValArrFstInt, RpArrScdInt, ValArrScdInt,
	RParenInitValIntExp, ValOrIdentInitValIntExp,
	LParenInt, ValIntOrIdent, RParenInt,
	IdentAfterReal,
	RpArrFstReal, ValArrFstReal, RpArrScdReal, ValArrScdReal,
	RParenInitValRealExp, ValOrIdentInitValRealExp,
	LParenReal, ValOrIdentReal, RParenReal,
	IdentAfterStr,
	IdentAfterCommaStr, LParenStr, StrExpect,
	RParenStr, ValIntOrIdentArray,
	ValIntOrIdentArraySec, RSBExpect, LParenWhile, RParenWhileExp,
	ValOrIdentWhileExp, RParenWhile, LBWhile, RBWhile, LParenGet, IdentGet,
	IdentGetComma, RParenGet, LParenPut, RParenPutExp, ValOrIdentPutExp,
	RParenPut,
	AssignExpect, IdentExpectAssign, RParenAssignExp, ValOrIdentAssignExp,
	LParenIf,
	RParenIfExp, ValOrIdentIfExp, RParenIf, LBIfExpect, RBIfExpect,
	LBElseExpect, RBElseExpect,
	RpArrFstExp, ValArrFstExp, RpArrScdExp, ValArrScdExp,
	RpArrFstAsgn, ValArrFstAsgn, RpArrScdAsgn, ValArrScdAsgn,
	RParenExp, ValOrIdentExp, LvalueReq,
	RParenProdExp, ValOrIdentProdExp,
	RParenSellFstExp, ValOrIdentSellFstExp,
	RParenSellScdExp, ValOrIdentSellScdExp,
	RParenBuyFstExp, ValOrIdentBuyFstExp,
	RParenBuyScdExp, ValOrIdentBuyScdExp,
	RParenBuildExp, ValOrIdentBuildExp,
	RParenAbuildExp, ValOrIdentAbuildExp,
	RParenPrintPlayerInfoExp, ValOrIdentPrintPlayerInfoExp,
	RParenPlayerProdExp, ValOrIdentPlayerProdExp,
	RParenPlayerRawExp, ValOrIdentPlayerRawExp,
	RParenPlayerFactExp, ValOrIdentPlayerFactExp,
	RParenPlayerMoneyExp, ValOrIdentPlayerMoneyExp,
	RParenPlayerAfactExp, ValOrIdentPlayerAfactExp,
	RParenPlayerActiveExp, ValOrIdentPlayerActiveExp,
	RParenDifBoughtExp, ValOrIdentDifBoughtExp,
	RParenDifSoldExp, ValOrIdentDifSoldExp,
	RParenLastBoughtExp, ValOrIdentLastBoughtExp,
	RParenLastSoldExp, ValOrIdentLastSoldExp,
	RParenLastDifBoughtExp, ValOrIdentLastDifBoughtExp,
	RParenLastDifSoldExp, ValOrIdentLastDifSoldExp
};

extern const char *TableOfWords[];
extern const char *TableOfErrors[];

#endif
