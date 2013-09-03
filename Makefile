shell: shell.o Single.o Slater.o
		g++ shell.o Single.o Slater.o -o shell -larmadillo -lboost_filesystem -lboost_system

shell.o: shell.cpp Single.h Slater.h
	g++ -c shell.cpp

Single.o: Single.cpp Single.h
	g++ -c Single.cpp

Slater.o: Slater.cpp Slater.h Single.h
	g++ -c Slater.cpp

clean:
	rm *.o
