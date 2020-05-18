all: compile archive

PROGRAM1=messages
PROGRAM2=stringParser

# create .o files
compile: $(PROGRAM1).h $(PROGRAM2).h
	g++ -std=c++17 -c $(PROGRAM1).cpp  $(PROGRAM2).cpp 

# create static library shared.a from .o files
archive:
	ar r sharedlib.a messages.o stringParser.o

clean:
	-rm *.o *.a
