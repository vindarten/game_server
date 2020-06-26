#include "lexeme.h"

Lexeme::Lexeme(char buf[], int &BufSize, int line, int num)
{
	LexNum = num;
	LineNum = line;
	if (LexNum == LexIdent || LexNum == LexError) {
		value = new char[BufSize + 1];
		for(int i = 0; i <= BufSize; i++)
			((char *)value)[i] = buf[i];
	} else if (LexNum == LexStr) {
		value = new char[BufSize - 1];
		for(int i = 0; i < BufSize - 2; i++)
			((char *)value)[i] = buf[i+1];
		((char *)value)[BufSize - 1] = 0;
	} else if (LexNum == LexValInt) {
		value = new long long(strtoll(buf, 0, 10));
	} else if (LexNum == LexValReal) {
		value = new double(strtod(buf, 0));
	}
	buf[0] = 0;
	BufSize = 0;
}

Lexeme::Lexeme(int line, int num)
{
	LexNum = num;
	LineNum = line;
}

void Lexeme::Print() 
{
	if (LexNum == LexIdent || LexNum == LexStr) {
		printf("%s", (char *)value);
	} else if (LexNum == LexValInt) {
		printf("%lld", *(long long *)value);
	} else if (LexNum == LexValReal) {
		printf("%f", *(double *)value);
	} else {
		printf("%s", TableOfWords[LexNum]);
	}
}

Lexeme::~Lexeme()
{
	if (LexNum == LexIdent || LexNum == LexStr)
		delete [] (char *)value;
	if (LexNum == LexValInt)
		delete (long long *)value;
	if (LexNum == LexValReal)
		delete (double *)value;
}
