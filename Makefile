CCC=mpic++
CFLAGS=-I"C:\msys64\ucrt64\include"
LDFLAGS= -L"C:\msys64\ucrt64\lib" -lfreeglut -lglu32 -lopengl32  -Wl,--subsystem,windows

SRC= main.cpp 
DEBUBFLAG=-g

OBJ= $(SRC:.cpp=.o)
BIN=./
EXEC=main

all: $(EXEC)

main: $(OBJ)
		$(CCC) -g -o $(BIN)$@ $^ $(LDFLAGS)

%.o: %.cpp
		$(CCC) -g -o $@ -c $< $(CFLAGS)


clean:
		rm -rf *.o

mrproper: clean
		rm -rf $(BIN)$(EXEC)
