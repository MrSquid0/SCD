.SUFFIXES:
.PHONY: x, pc1, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors

x: monitor_em_exe
	./$<

pclifo: prodcons1_su_LIFO_exe
	./$<

pcfifo: prodcons1_su_FIFO_exe
	./$<

%_exe: %.cpp scd.cpp scd.h
	$(compilador) $(opcionesc)  -o $@ $< scd.cpp

clean:
	rm -f *_exe
