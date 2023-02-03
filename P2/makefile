.SUFFIXES:
.PHONY: x, pc1, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors

pcmlifo: prodcons_mu_LIFO_exe
	./$<

pcmfifo: prodcons_mu_FIFO_exe
	./$<

fu: fumadores_exe
	./$<

le: lectoresEscritores_exe
	./$<

%_exe: %.cpp scd.cpp scd.h
	$(compilador) $(opcionesc)  -o $@ $< scd.cpp

clean:
	rm -f *_exe
