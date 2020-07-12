#include "automat.h"

int Automat::Letter(char c)
{
	return (c>='a' && c<='z') || (c>='A' && c<='Z');
}

int Automat::Digit(char c)
{
	return c>='0' && c<='9';
}

int Automat::EndLine(char c)
{
	return c=='\n';
}

int Automat::Space(char c)
{
	return c==' ' || c=='\t';
}

int Automat::Brace(char c)
{
	return c=='(' || c==')' || c=='[' || c==']' || c=='{' || c=='}';
}

int Automat::Compare(char c)
{
	return c=='>' || c=='<' || c=='=';
}

int Automat::Arithmetic(char c)
{
	return c=='+' || c=='-' || c=='*' || c=='/';
}

int Automat::Single(char c)
{
	return Brace(c) || Arithmetic(c) || c==';' || c==',' || c=='!';
}

int Automat::Other(char c)
{
	return c=='@' || c=='#' || c=='$' || c=='%' || c=='?' || c=='&';
}

int Automat::All(char c)
{
	return Letter(c)||Digit(c)||Compare(c)||Single(c)||Other(c)||Space(c); 
}

int Automat::Delimiter(char c)
{
	return EndLine(c)||Compare(c)||Single(c)||Space(c)||Digit(c)||Letter(c);
}

Lexeme *Automat::FeedChar(char c)
{
	if (state == S) {
		return StateSResend(c);
	} else if (state == Error) {
		AddBuf(c);
		return new Lexeme(buf, BufSize, line, LexError);
	}
	ChangeState(c);
	if (state == SResend) {
		return StateSResend(c);
	} else {
		AddBuf(c);
		if (state == S) {
			return StateS(c);
		} else if (state == Error) {
			return new Lexeme(buf, BufSize, line, LexError);
		}
	}
	return NULL;
}

Lexeme *Automat::StateSResend(char c)
{
	state = H;
	lex = new Lexeme(buf, BufSize, line, GetLexNum());
	if (c == '\n')
		line++;
	ChangeState(c);
	AddBuf(c);
	if ((*lex).Empty()) {
		delete lex;
		lex = NULL;
	}
	return lex;
}

Lexeme *Automat::StateS(char c)
{
	state = H;
	lex = new Lexeme(buf, BufSize, line, GetLexNum());
	if (c == '\n')
		line++;
	if ((*lex).Empty()) {
		delete lex;
		lex = NULL;
	}
	return lex;
}

void Automat::ChangeState(char c)
{
	if (state == H) {
		StateH(c);
	} else if (state == String) {
		StateString(c);
	} else if (state == Ident) {
		StateIdent(c);
	} else if (state == Int) {
		StateInt(c);
	} else if (state == Real) {
		StateReal(c);
	} else if (state == Equal) {
		StateEqual(c);
	} else if (state == LessGreater) {
		StateLessGreater(c);
	} else if (state == Comment) {
		StateComment(c);
	}
}

void Automat::StateH(char c)
{
	if (Digit(c)) {
		state = Int;
	} else if (c == '\"') {
		state = String;
	} else if (Letter(c)) {
		state = Ident;
	} else if (c == '=') {
		state = Equal;
	} else if (c == '>' || c == '<') {
		state = LessGreater;
	} else if (c == '\\') {
		state = Comment;
	} else if (EndLine(c) || Single(c) || Space(c)) {
		state = S;
	} else {
		state = Error;
	}
}

void Automat::StateInt(char c)
{
	if (Digit(c)) {
	} else if (c == '.') {
		state = Real;
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateReal(char c)
{
	if (Digit(c)) {
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateString(char c)
{
	if (c == '\"') {
		state = S;
	} else if (All(c)) {
	} else {
		state = Error;
	}
}

void Automat::StateIdent(char c)
{
	if (Letter(c) || Digit(c)) {
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateEqual(char c)
{
	if (c == '=' || c == '!') {
		state = S;
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateLessGreater(char c)
{
	if (c == '=') {
		state = S;
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateComment(char c)
{
	if (EndLine(c)) {
		state = S;
	}
}

void Automat::AddBuf(char c)
{
	if (((c != ' ' && c != '\t') || BufSize != 0) && !EndLine(c)) {
		buf[BufSize++] = c;
		buf[BufSize] = 0;
	}
}

int Automat::SearchPoint()
{
	for(int i = 0; i < BufSize; i++)
		if (buf[i] == '.')
			return 1;
	return 0;
}

int Automat::GetLexNum()
{
	int i = 0;
	if (Digit(buf[0])) { 
		if (SearchPoint()) {
			return LexValReal;
		} else {
		return LexValInt;
		}
	} else if (buf[0] == '\\') {
		return LexEmpty;
	} else if (buf[0] == '"') {
		return LexStr;
	}
	while(TableOfWords[i] != NULL) {
		if (!MyStrcmp(buf, TableOfWords[i]))
			return i;
		i++;
	}
	if (Letter(buf[0]))
		return LexIdent;
	return LexError;
}
