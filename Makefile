SRCMODULES = lexeme.cpp tables.cpp automat.cpp GameInfo.cpp IpnEx.cpp IpnElem.cpp Ipn.cpp StackOperations.cpp Parser.cpp
OBJMODULES = $(SRCMODULES:.c =.o)
CFLAGS = -g -Wall

%.o: %.c %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

robot: robot.cpp $(OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o $@

-include dependence.mk

dependence.mk: $(SRCMODULES)
	$(CXX) -MM $^ > $@
