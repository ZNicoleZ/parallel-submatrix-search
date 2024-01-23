#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include "structs.h"

// Master Process Utility Functions

int readPictures(Picture** pictures, int* size, FILE* fp);

void printPictures(Picture** pictures, int size);

int readObjects(Object** objects, int* size, FILE* fp);

void printObjects(Object** objects, int size);

int updatePicture(Picture* pictures, int num_pictures, int objID, int objI, int objJ, int picID);

int printPicturesDetails(Picture* pictures, int num_pictures, FILE *fp);

int printPicturesDetailsToFile(Picture* pictures, int num_pictures, const char* file_name);

void freeResources(Picture** pictures, int num_pictures, Object** objects, int num_objects);


// MPI Sending and Receiving

int sendTask(int taskData[4], int** pictureMat, int** objectMat, Picture* pictures, int picIndex, Object* objects, int objIndex, int pDest, int tag);

int receiveTask(int taskData[4], int** pictureMat, int** objectMat, double match_value, int result[4], MPI_Status *status, int *tag);


// Handle CUDA

int copyDataToGPU(int* picMat, int picDim, int* objMat, int objDim, int** dev_pic, int** dev_obj, int** dev_ind);

int compareThreadMatrixOnGPU(int *d_pic, int picDim, int *d_obj, int objDim, double matchValue, int* d_ind, int fromRow , int toRow, int fromCol, int toCol);

int freeCudaData(int *d_pic, int *d_obj, int* d_ind);
