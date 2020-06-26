#include "MyString.h"

int MyStrlen(const char *s)
{
	int i = 0;
	while(s[i] != 0) {
		i++;
	}
	return i;
}

int MyStrcmp(const char *s1, const char *s2)
{
	int i = 0;
	while(s1[i] == s2[i] && s1[i] != 0) {
		i++;
	}
	return !(s1[i] == s2[i]);
}

char *MyStrdup(const char *s)
{
	char *n = new char[MyStrlen(s)+1];
	for(int i = 0; i <= MyStrlen(s); i++) {
		n[i] = s[i];
	}
	return n;
}
