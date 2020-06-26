lexeme.o: lexeme.cpp lexeme.h tables.h
tables.o: tables.cpp tables.h
automat.o: automat.cpp automat.h lexeme.h tables.h
GameInfo.o: GameInfo.cpp GameInfo.h
IpnEx.o: IpnEx.cpp IpnEx.h
IpnElem.o: IpnElem.cpp IpnElem.h IpnEx.h GameInfo.h
Ipn.o: Ipn.cpp Ipn.h IpnElem.h IpnEx.h GameInfo.h
StackOperations.o: StackOperations.cpp StackOperations.h Ipn.h IpnElem.h \
 IpnEx.h GameInfo.h lexeme.h tables.h
Parser.o: Parser.cpp Parser.h Ipn.h IpnElem.h IpnEx.h GameInfo.h \
 StackOperations.h lexeme.h tables.h automat.h
