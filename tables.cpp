#include "tables.h"

const char *TableOfWords[] = 
{
	"", "", "", "", "", "birth", "death", "die", "int", "real", "string",
	"if", "else", "while", "put", "get", ",", ";", "=",
	"{", "}", "[", "]", "(", ")",
	"or", "and", "<", "<=", ">", ">=", "==", "=!", "+", "-", "*", "/", "!",
	"prod", "sell", "buy", "build", "abuild", "upgrade",
	"MinRawPrice", "MaxProdPrice", "MaxRaw", "MaxProd", "MaxNum", "Month",
	"MyProd", "MyRaw", "MyFact", "MyMoney", "MyAfact", "MyNum",
	"PlayerProd", "PlayerRaw", "PlayerFact", "PlayerMoney", "PlayerAfact",
	"PlayerActive",
	"PlayerDifBought", "PlayerDifSold",
	"PlayerLastDifBought", "PlayerLastDifSold",
	"PlayerLastBought", "PlayerLastSold",
	"PrintMyInfo", "PrintMarketInfo", "PrintPlayerInfo",
	0
};

const char *TableOfErrors[] = 
{
	"",
	"\"birth\" expected in the beginning of the program",
	"\"death\" expected in the end of the program",
	"lexeme after end of the program",
	"\";\" expected in the end of expression",
	"identifier expected in description of integer",
	"\")\" expected in the condition of first dimension of array "
	"in description of integer",
	"identifier or int or real value expected in the condition of "
	"first dimension of array in description of integer",
	"\")\" expected in the condition of second dimension of array "
	"in description of integer",
	"identifier or int or real value expected in the condition of "
	"second dimension of array in description of integer",
	"\")\" expected in the condition of initial value "
	"in description of integer",
	"identifier or int or real value expected in the condition of "
	"initial value in description of integer",
	"all variables must be initialized: "
	"\"(\" expected after identifier in description of integer",
	"identifier or int value expected after \"(\" in description of integer",
	"\")\" expected after initial value in description of integer",
	"identifier expected in description of real",
	"\")\" expected in the condition of first dimension of array "
	"in description of real",
	"identifier or int or real value expected in the condition of "
	"first dimension of array in description of real",
	"\")\" expected in the condition of second dimension of array "
	"in description of real",
	"identifier or int or real value expected in the condition of "
	"second dimension of array in description of real",
	"\")\" expected in the condition of initial value "
	"in description of real",
	"identifier or int or real value expected in the condition of "
	"initial value in description of real",
	"all variables must be initialized: "
	"\"(\" expected after identifier in description of real",
	"identifier or real or int value expected after \"(\" "
	"in description of real",
	"\")\" expected after initial value in description of real",
	"identifier expected after \"string\"",
	"identifier expected after \",\" in description of string",
	"all variables must be initialized: "
	"\"(\" expected after identifier in description of string",
	"string expected after \"(\" in description of string",
	"\")\" expected after initial value in description of string",
	"identifier or int value expected after \"[\" in description of array",
	"identifier or int value expected after \",\" in description of array",
	"\"]\" expected after the dimension in the end of description of array",
	"\"(\" expected after \"while\"",
	"\")\" expected in the condition of \"while\"",
	"identifier or int or real value expected in the condition of \"while\"",
	"\")\" expected after the condition of \"while\"",
	"\"{\" expected after \")\" in the beginning of \"while\" body",
	"\"}\" expected in the end of \"while\" body",
	"\"(\" expected after \"get\"",
	"identifier expected after \"(\" as the argument of \"get\"",
	"identifier expected after \",\" as the argument of \"get\"",
	"\")\" expected after the argument of \"get\"",
	"\"(\" expected after \"put\"",
	"\")\" expected in the argument of \"put\"",
	"identifier or int or real value expected in the argument of \"put\"",
	"\")\" expected after the argument of \"put\"",
	"\"=\" expected after identifier in the assignment",
	"identifier expected after \":=\" in the assignment",
	"\")\" expected in the assignment",
	"identifier or int or real value expected in the assignment",
	"\"(\" expected after \"if\"",
	"\")\" expected in the condition of \"if\"",
	"identifier or int or real value expected in the condition of \"if\"",
	"\")\" expected after the condition of \"if\"",
	"\"{\" expected after \")\" in the beginning of \"if\" body",
	"\"}\" expected in the end of \"if\" body",
	"\"{\" expected after \"else\" in the beginning of \"else\" body",
	"\"}\" expected in the end of \"else\" body",
	"\")\" expected in the condition of first dimension of array "
	"in description of expression",
	"identifier or int or real value expected in the condition of "
	"first dimension of array in description of expression",
	"\")\" expected in the condition of second dimension of array "
	"in description of expression",
	"identifier or int or real value expected in the condition of "
	"second dimension of array in description of expression",
	"\")\" expected in the condition of first dimension of array "
	"in description of assignment",
	"identifier or int or real value expected in the condition of "
	"first dimension of array in description of assignment",
	"\")\" expected in the condition of second dimension of array "
	"in description of assignment",
	"identifier or int or real value expected in the condition of "
	"second dimension of array in description of assignment",
	"\")\" expected in the expression",
	"identifier or int or real value expected in the expression",
	"lvalue required as left operand of assignment",
	"\")\" expected in the argument of \"prod\"",
	"identifier or int value expected in the argument of \"prod\"",
	"\")\" expected in the first argument of \"sell\"",
	"identifier or int value expected in the first argument of \"sell\"",
	"\")\" expected in the second argument of \"sell\"",
	"identifier or int value expected in the second argument of \"sell\"",
	"\")\" expected in the first argument of \"buy\"",
	"identifier or int value expected in the first argument of \"buy\"",
	"\")\" expected in the second argument of \"buy\"",
	"identifier or int value expected in the second argument of \"buy\""
	"\")\" expected in the argument of \"build\"",
	"identifier or int value expected in the argument of \"build\"",
	"\")\" expected in the argument of \"abuild\"",
	"identifier or int value expected in the argument of \"abuild\"",
	"\")\" expected in the argument of \"PrintPlayerInfo\"",
	"identifier or int value expected in the argument of \"PrintPlayerInfo\"",
	"\")\" expected in the argument of \"PlayerProd\"",
	"identifier or int value expected in the argument of \"PlayerProd\"",
	"\")\" expected in the argument of \"PlayerRaw\"",
	"identifier or int value expected in the argument of \"PlayerRaw\"",
	"\")\" expected in the argument of \"PlayerFact\"",
	"identifier or int value expected in the argument of \"PlayerFact\"",
	"\")\" expected in the argument of \"PlayerMoney\"",
	"identifier or int value expected in the argument of \"PlayerMoney\"",
	"\")\" expected in the argument of \"PlayerAfact\"",
	"identifier or int value expected in the argument of \"PlayerAfact\"",
	"\")\" expected in the argument of \"PlayerActive\"",
	"identifier or int value expected in the argument of \"PlayerActive\"",
	"\")\" expected in the argument of \"PlayerDifBought\"",
	"identifier or int value expected in the argument of \"PlayerDifBought\"",
	"\")\" expected in the argument of \"PlayerDifSold\"",
	"identifier or int value expected in the argument of \"PlayerDifSold\"",
	"\")\" expected in the argument of \"PlayerLastBought\"",
	"identifier or int value expected in the argument of \"PlayerLastBought\"",
	"\")\" expected in the argument of \"PlayerLastSold\"",
	"identifier or int value expected in the argument of \"PlayerLastSold\"",
	"\")\" expected in the argument of \"PlayerLastDifBought\"",
	"identifier or int value expected in the argument of \"PlayerLastDifBought\"",
	"\")\" expected in the argument of \"PlayerLastDifSold\"",
	"identifier or int value expected in the argument of \"PlayerLastDifSold\""
};
