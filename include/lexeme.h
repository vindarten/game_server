#ifndef LEXEME_H
#define LEXEME_H

#include <stdio.h>
#include <stdlib.h>
#include "tables.h"

class Lexeme {
	void *value;
	int LexNum, LineNum;
public:
	Lexeme(): value(0), LexNum(LexEmpty), LineNum(0) {}
	Lexeme(char buf[], int &BufSize, int line, int num);
	Lexeme(int line, int num);
	~Lexeme();
	int Empty() { return !LexNum; }
	int CheckCorrect() { return LexNum != LexError; } 
	int GetLexNum() { return LexNum; } 
	int GetLineNum() { return LineNum; }
	char *GetName() { return (char *)value; }
	long long GetInt() { return *((long long *)value); }
	double GetReal() { return *((double *)value); }
	char *GetString() { return (char *)value; }
	void Print(); 
};
#endif
