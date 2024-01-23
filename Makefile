build:
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -c cFunctions.c -o cFunctions.o
	nvcc -I./Common  -gencode arch=compute_61,code=sm_61 -c cudaFunctions.cu -o cudaFunctions.o
	mpicxx -fopenmp -o ParallelProject  main.o cFunctions.o cudaFunctions.o  -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -lcudart
	

clean:
	rm -f *.o ./ParallelProject

run:
	mpiexec -np 4 ./ParallelProject

runOn2:
	mpiexec -np 8 -machinefile  mf  -map-by  node  ./ParallelProject
	
