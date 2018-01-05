objects = Filesys.o qu.o fu.o
Filesys : $(objects)
	g++ -o Filesys $(objects)
Filesys.o : Filesys.cpp Filesys.h
	g++ -c filesys.cpp
qu.o : qu.cpp qu.h
	g++ -c qu.cpp
fu.o : fu.cpp fu.h
	g++ -c fu.cpp
clean :
	rm Filesys $(objects)

