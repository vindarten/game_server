SRCMODULES = lexeme.cpp tables.cpp automat.cpp GameInfo.cpp IpnEx.cpp IpnElem.cpp Ipn.cpp StackOperations.cpp Parser.cpp MyString.cpp
OBJMODULES = $(SRCMODULES:.cpp=.o)
CXXFLAGS = -g -Wall

robot: robot.cpp $(OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include dependence.mk
endif

dependence.mk: $(SRCMODULES)
	$(CXX) -MM $^ > $@

clean:
	rm -f *.o robot
