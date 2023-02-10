.SUFFIXES:
.PHONY:    pc,pc2,pcm,fp,f,fi,fc,clean

compilador := mpicxx 
flagsc     := -std=c++11 -g -Wall

pc: prodcons_mpi_exe
	mpirun -oversubscribe -np  3 ./$<

pc2fifo: prodcons2-FIFO_mpi_exe
	mpirun -oversubscribe  -np  3 ./$<

pc2lifo: prodcons2-LIFO_mpi_exe
	mpirun -oversubscribe  -np  3 ./$<

pcmfifo: prodcons2-mu-FIFO_mpi_exe
	mpirun -oversubscribe  -np  10 ./$<

pcmlifo: prodcons2-mu-LIFO_mpi_exe
	mpirun -oversubscribe  -np  10 ./$<

fp: filosofos-plantilla_mpi_exe
	mpirun -oversubscribe  -np 10 ./$<

fi: filosofos-interb_mpi_exe
	mpirun -oversubscribe  -np 10 ./$<

f: filosofos_mpi_exe
	mpirun -oversubscribe  -np 10 ./$<

fc: filosofos-cam_mpi_exe
	mpirun -oversubscribe  -np 11 ./$<

%_mpi_exe: %.cpp
	$(compilador) $(flagsc) -o $@ $<

clean:
	rm -rf *_exe *.dSYM
