all: compile archive

PROGRAM1=messages
PROGRAM2=stringParser
PROGRAM3=ReadLine

# create .o files
compile: $(PROGRAM1).h $(PROGRAM2).h $(PROGRAM3).h
	g++ -std=c++17 -c $(PROGRAM1).cpp $(PROGRAM2).cpp $(PROGRAM3).cpp 

# create static library shared.a from .o files
archive:
	ar r sharedlib.a $(PROGRAM1).o $(PROGRAM2).o $(PROGRAM3).o

clean:
	-rm *.o *.a
