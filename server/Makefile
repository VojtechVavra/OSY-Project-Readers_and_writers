CXX=g++
CXXFLAGS= -std=c++17 -Wall -Wextra -g -pthread
CXXFLAGS2= -pedantic -Wall -Wextra -g -pthread

PROGRAM=threadedServer
NAME1=threadedServer
NAME2=../shared/stringParser
STATICLIB1=../shared/libshared

$(NAME1): $(NAME1).cpp
	$(CXX) $(CXXFLAGS) $(NAME1).cpp $(STATICLIB1).a -o $(NAME1) 


#threadedServer: ../shared/stringParser.cpp threadedServer.cpp
	#g++ -o -pthread threadedServer threadedServer.cpp ../shared/stringParser.cpp

clean:
	rm -f $(NAME1)   # smaže binární soubor
