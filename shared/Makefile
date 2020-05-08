
all: compile archive

# create .o files
compile:
	g++ -std=c++17 -c stringParser.cpp

# create static library shared.a from .o files
archive:
	ar r libshared.a stringParser.o

clean:
	-rm *.o *.a
