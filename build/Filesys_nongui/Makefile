SRC = ../../src/
BUI = ./
objects = $(BUI)Filesys.o $(BUI)qu.o $(BUI)fu.o
$(BUI)Filesys : $(objects)
	g++ -o $(BUI)Filesys $(objects)
$(BUI)Filesys.o : $(SRC)Filesys.cpp $(SRC)Filesys.h
	g++ -c $(SRC)filesys.cpp
$(BUI)qu.o : $(SRC)qu.cpp $(SRC)qu.h
	g++ -c $(SRC)qu.cpp
$(BUI)fu.o : $(SRC)fu.cpp $(SRC)fu.h
	g++ -c $(SRC)fu.cpp
clean :
	rm $(BUI)Filesys $(objects)

