#ifndef AUTOMAT_H
#define AUTOMAT_H

#include "lexeme.h"
#include "tables.h"
#include "MyString.h"

class Automat {
	enum {
		H, String, Ident, Int, Real, Equal, LessGreater, 
		Comment, MultiAssign, Error, S, SResend
	};
	enum {MaxBuf = 4096};
	Lexeme *lex;
	char buf[MaxBuf];
	int BufSize, state, line;
	Lexeme *StateS(char c);
	Lexeme *StateSResend(char c);
	void ChangeState(char c);
	void StateH(char c);
	void StateString(char c);
	void StateIdent(char c);
	void StateInt(char c);
	void StateReal(char c);
	void StateEqual(char c);
	void StateLessGreater(char c);
	void StateComment(char c);
	void StateMultiAssign(char c);
	void AddBuf(char c);
	int SearchPoint();
	int GetLexNum();
	int Letter(char c); 
	int Digit(char c);
	int Space(char c);
	int EndLine(char c);
	int Brace(char c);
	int Arithmetic(char c);
	int Compare(char c);
	int Single(char c);
	int Other(char c);
	int All(char c);
	int Delimiter(char c);
public:
	Automat(): lex(0), BufSize(0), state(H), line(1) { buf[0] = 0; }
	Lexeme *FeedChar(char c);
};
#endif
