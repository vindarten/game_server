lexeme.o: lexeme.cpp lexeme.h tables.h
tables.o: tables.cpp tables.h
automat.o: automat.cpp automat.h lexeme.h tables.h MyString.h
GameInfo.o: GameInfo.cpp GameInfo.h MyString.h
IpnEx.o: IpnEx.cpp IpnEx.h MyString.h
IpnElem.o: IpnElem.cpp IpnElem.h MyString.h IpnEx.h GameInfo.h
Ipn.o: Ipn.cpp Ipn.h IpnElem.h MyString.h IpnEx.h GameInfo.h
StackOperations.o: StackOperations.cpp StackOperations.h Ipn.h IpnElem.h \
 MyString.h IpnEx.h GameInfo.h lexeme.h tables.h
Parser.o: Parser.cpp Parser.h Ipn.h IpnElem.h MyString.h IpnEx.h \
 GameInfo.h StackOperations.h lexeme.h tables.h automat.h
MyString.o: MyString.cpp MyString.h
