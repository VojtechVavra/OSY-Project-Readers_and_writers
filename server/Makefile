CXX=g++
CXXFLAGS= -std=c++17 -Wall -Wextra -g -pthread -lrt 
#CXXFLAGS2= -Wall -Wextra -g -lrt 

PROGRAM1=thread_server
PROGRAM2=fork_server

STATICLIB=../shared/sharedlib

all: $(PROGRAM1) $(PROGRAM2)

$(PROGRAM1): $(PROGRAM1).cpp
	$(CXX) $(PROGRAM1).cpp $(STATICLIB).a -o $(PROGRAM1) $(CXXFLAGS) 

$(PROGRAM2): $(PROGRAM2).cpp
	$(CXX) $(PROGRAM2).cpp $(STATICLIB).a -o $(PROGRAM2) $(CXXFLAGS) 

clean:
	rm -f $(PROGRAM1) $(PROGRAM2)  # smaže binární soubor
