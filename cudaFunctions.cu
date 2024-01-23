#include <cuda_runtime.h>
#include <helper_cuda.h>
#define BLOCKS 8
#define THREADS 1024

__device__ double calcDiff(int p, int o) {

	// utility function for calculating difference between Picture element and an Object element
	
	double dif = p-o;
	dif = abs(dif);
	dif = dif/p;
	return dif;
}

__device__ double calcMatrixMatch(int* picMat, int picDim, int* objMat, int objDim, int i, int j) {

	// utility function for calculating the overall matching value of a picture and an object in position i,j
	
	double match_sum = 0;
	for(int r = 0; r<objDim; r++){
		for(int c=0; c<objDim; c++){
			if(r+i < picDim && c+j < picDim)
				match_sum += calcDiff(picMat[(i+r)*picDim + j+c], objMat[r*objDim + c]);
			else
				return -1;
		}
	}

	return match_sum/(objDim*objDim);
}


__global__ void kernelCheckMatrixRangeMatch(int *d_pic, int picDim, int *d_obj, int objDim, double matchValue, int* d_ind, int fromRow , int toRow, int fromCol, int toCol) {
	
	int i = blockIdx.x * blockDim.x + threadIdx.x;	// set a "global" thread index for each thread
    	int numRows = toRow - fromRow;			// set row range
	int numCols = toCol - fromCol;			// set column range
	
	if(numRows <= 0 || numCols <= 0) {
    		return;
    	}
    	
    	int row = (i)/(numRows) + fromRow;		// define position's row to check
	int col = (i)%(numRows) + fromCol;		// define position's column to check

	if (row >= fromRow && row <= toRow && col >= fromCol && col <= toCol) {	// if the position is in the given range, calculate matching value
		
		double match = calcMatrixMatch(d_pic, picDim, d_obj, objDim, row, col);
		
		if(match<=matchValue && match!=-1){	// if object was found, set the found index in the result pointer (d_ind)
			*d_ind = (row*picDim + col);
			return;
		}
	}

}

int copyDataToGPU(int* picMat, int picDim, int* objMat, int objDim, int** dev_pic, int** dev_obj, int** dev_ind) {

	cudaError_t err = cudaSuccess;

	size_t picSize = picDim * picDim * sizeof(int);
	size_t objSize = objDim * objDim * sizeof(int);

	// Allocate memory on GPU to copy the data from the host
	int *d_pic;
	err = cudaMalloc((void **)&d_pic, picSize);
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to allocate device memory (picture) - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	int *d_obj;
	err = cudaMalloc((void **)&d_obj, objSize);
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to allocate device memory (object) - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	
	int h_ind = -1;
	int* d_ind;
	err = cudaMalloc((void**)&d_ind, sizeof(int));
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to allocate device memory (index) - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	// Copy data from host to the GPU memory
	err = cudaMemcpy(d_pic, picMat, picSize, cudaMemcpyHostToDevice);
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to copy data from host to device (picture) - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	err = cudaMemcpy(d_obj, objMat, objSize, cudaMemcpyHostToDevice);
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to copy data from host to device (object) - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
    
	err = cudaMemcpy(d_ind, &h_ind, sizeof(int), cudaMemcpyHostToDevice);
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to copy data from host to device (index) - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	
	*dev_pic = d_pic;
	*dev_obj = d_obj;
	*dev_ind = d_ind;
	
	return 0;
}

int compareThreadMatrixOnGPU(int *d_pic, int picDim, int *d_obj, int objDim, double matchValue, int* d_ind, int fromRow , int toRow, int fromCol, int toCol) {

	cudaError_t err = cudaSuccess;
	int h_ind;
	
	int rows = toRow - fromRow;
	int cols = toCol - fromCol;
	int size = (rows + 1) * (cols + 1);
	int blocks = (size + THREADS - 1) / THREADS;	

	// Launch the Kernel
	kernelCheckMatrixRangeMatch<<<blocks, THREADS>>>(d_pic, picDim, d_obj, objDim, matchValue, d_ind, fromRow , toRow, fromCol, toCol);
	err = cudaGetLastError();

	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to launch 'kernelCheckMatrixRangeMatch()' -  %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	// Copy the  result from GPU to the host memory.
	err = cudaMemcpy(&h_ind, d_ind, sizeof(int), cudaMemcpyDeviceToHost);
	if (err != cudaSuccess) {
		fprintf(stderr, "Failed to copy data from device to host - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	
	return h_ind;
	
}


int freeCudaData(int *d_pic, int *d_obj, int* d_ind) {

	cudaError_t err = cudaSuccess;
	
	// Free allocated memory on GPU
	if (cudaFree(d_pic) != cudaSuccess) {
		fprintf(stderr, "Failed to free device data (picture) - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	if (cudaFree(d_obj) != cudaSuccess) {
		fprintf(stderr, "Failed to free device data (object) - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	
	if (cudaFree(d_ind) != cudaSuccess) {
		fprintf(stderr, "Failed to free device data (index) - %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	
	return 0;

}


