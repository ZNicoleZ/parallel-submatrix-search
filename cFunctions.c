#include "cFunctions.h"


int readPictures(Picture** pictures, int* size, FILE* fp) {

	// Read the size of the data
	fscanf(fp, "%d", size);

	// Allocate memory for the array
	*pictures = (Picture*) malloc(*size * sizeof(Picture));
	if(*pictures == NULL){
		printf("failed to allocate pictures array\n");
		return 1;
	}

	// Read the data into the array
	for (int i = 0; i < *size; i++) {
		fscanf(fp, "%d %d", &(*pictures)[i].id, &(*pictures)[i].dimension);
		(*pictures)[i].num_found = 0;

		int picDim = (*pictures)[i].dimension;

		// Allocate memory for the matrix of each picture
		(*pictures)[i].matrix = (int*) malloc(picDim * picDim * sizeof(int));
		if((*pictures)[i].matrix == NULL){
			printf("failed to allocate pictures matrix\n");
			return 1;
		}

		// Read the data of each picture
		for (int j = 0; j < picDim*picDim; j++) {
			fscanf(fp, "%d", &((*pictures)[i].matrix[j]));
		}
	}

	return 0;
}

int readObjects(Object** objects, int* size, FILE* fp) {

	// Read the size of the data
	fscanf(fp, "%d", size);

	// Allocate memory for the array
	*objects = (Object*) malloc(*size * sizeof(Object));
	if(*objects == NULL){
		printf("failed to allocate objects array\n");
		return 1;
	}

	// Read the data into the array
	for (int i = 0; i < *size; i++) {
		fscanf(fp, "%d %d", &(*objects)[i].id, &(*objects)[i].dimension);
		// Allocate memory for the data of each picture
		(*objects)[i].matrix = (int*) malloc((*objects)[i].dimension * (*objects)[i].dimension * sizeof(int));
		if((*objects)[i].matrix == NULL){
			printf("failed to allocate objects matrix\n");
			return 1;
		}
	
		// Read the data of each picture
		for (int j = 0; j < (*objects)[i].dimension; j++) {
			for (int k = 0; k < (*objects)[i].dimension; k++) {
				fscanf(fp, "%d", &(*objects)[i].matrix[j*(*objects)[i].dimension + k]);
			}
		}
	}

	return 0;
}

void printPictures(Picture** pictures, int size) {

	for(int i=0; i<size; i++){
		printf("   picture ID: %d,  dimension:%d\n",(*pictures)[i].id, (*pictures)[i].dimension);
	}
	
}

void printObjects(Object** objects, int size) {

	for(int i=0; i<size; i++){
		printf("   object ID: %d,  dimension:%d\n",(*objects)[i].id, (*objects)[i].dimension);
	}
}

int updatePicture(Picture* pictures, int num_pictures, int objID, int objI, int objJ, int picID) {
	
	for(int pic=0; pic<num_pictures; pic++){ 				
		if(pictures[pic].id == picID && pictures[pic].num_found<3){	// search for the picture and update found objects (if less than 3 were found)
			pictures[pic].objects_found[pictures[pic].num_found][0] = objID;
			pictures[pic].objects_found[pictures[pic].num_found][1] = objI;
			pictures[pic].objects_found[pictures[pic].num_found][2] = objJ;
			pictures[pic].num_found++;
			return 0;
		}
	}
	
	return -1;
}

int printPicturesDetails(Picture* pictures, int num_pictures, FILE *fp) {

	for(int i=0; i<num_pictures; i++){
		fprintf(fp, "Picture %d: ",pictures[i].id);
		
		// if 3 differen objects were found in the picture, print it's details
		if(pictures[i].num_found == 3){	
			fprintf(fp, "found Objects:  ");
			for(int num=0; num<pictures[i].num_found; num++){
				fprintf(fp, "%d Position(%d,%d)",pictures[i].objects_found[num][0], pictures[i].objects_found[num][1], pictures[i].objects_found[num][2]);
				if(num != pictures[i].num_found-1) fprintf(fp, " ; ");
			}
		} 
		
		// if no 3 different objects were found, print so
		else {
			fprintf(fp, "No three different Objects were found");
		}
		fprintf(fp, "\n");
	}
	
	return 0;
}

int printPicturesDetailsToFile(Picture* pictures, int num_pictures, const char* file_name) {

	FILE *fp;
	
	fp = fopen(file_name, "w"); // open the file in write mode
	if (fp == NULL) {
		printf("Failed to open the file...\n");
		return 1;
	}
	
	// print the details to the opened output file
	printPicturesDetails(pictures, num_pictures, fp);	
	
	// close the file
	fclose(fp);

	printf("\n---- output file was added ----\n");
	
	return 0;
}

void freeResources(Picture** pictures, int num_pictures, Object** objects, int num_objects) {

	// Free the memory for each picture
	for (int i = 0; i < num_pictures; i++) {
	    free((*pictures)[i].matrix);
	}
	
	// Free the memory for the pictures array
	free(*pictures);
	
	// Free the memory for each picture
	for (int i = 0; i < num_objects; i++) {
	    free((*objects)[i].matrix);
	}
	
	// Free the memory for the pictures array
	free(*objects);
}

int sendTask(int taskData[4], int** pictureMat, int** objectMat, Picture* pictures, int picIndex, Object* objects, int objIndex, int pDest, int tag) {

	// groupe Picture-Object couple data
	taskData[0] = pictures[picIndex].id;
	taskData[1] = pictures[picIndex].dimension;
	taskData[2] = objects[objIndex].id;
	taskData[3] = objects[objIndex].dimension;
	
	int picDim = pictures[picIndex].dimension;
	int objDim = objects[objIndex].dimension;

	// send the dimensions of the matrix to process pDest
	MPI_Send(taskData, 4, MPI_INT, pDest, tag, MPI_COMM_WORLD);

	// send the picture matrix to process p
	*pictureMat = pictures[picIndex].matrix;
	MPI_Send(*pictureMat, picDim*picDim, MPI_INT, pDest, tag, MPI_COMM_WORLD);

	// send the object matrix to process p
	*objectMat = objects[objIndex].matrix;
	MPI_Send(*objectMat, objDim*objDim, MPI_INT, pDest, tag, MPI_COMM_WORLD);
	
	return 0;
}

int receiveTask(int taskData[4], int** pictureMat, int** objectMat, double match_value, int result[4], MPI_Status *status, int *tag) {

	int terminate = 1;	// terminate tag = 1, work tag = 0
	int ind;
	
	// receive the Picture-Object couple data
	MPI_Recv(taskData, 4, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, status);

	*tag = (*status).MPI_TAG;
	if(*tag != terminate){		// check if it's a work task or termination task

		int picDim = taskData[1];
		int objDim = taskData[3];
		ind = -1;
		
		// receive the picture matrix
		*pictureMat = (int *)malloc(picDim * picDim * sizeof(int));
		MPI_Recv(*pictureMat, picDim*picDim, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		
		// receive the object matrix
		*objectMat = (int *)malloc(objDim * objDim * sizeof(int));
		MPI_Recv(*objectMat, objDim*objDim, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		
		int num_indexes = (picDim-objDim);	// dimension of the picture submatrix that contains valid possible positions for the object
		int numOfThreads = 4;			// 4 threads for processing the 4 quarters of the matrix
		int *dev_pic, *dev_obj, *dev_ind;	// hold device pointers for GPU functions
		
		copyDataToGPU(*pictureMat, picDim, *objectMat, objDim, &dev_pic, &dev_obj, &dev_ind);	// the threads can use shared memory so we don't need to copy the data for each of them
		
		int tid, fromRow, toRow, fromCol, toCol;
		int object_is_found = 0;
		if(picDim != 0 && objDim != 0) {
			#pragma omp parallel num_threads(numOfThreads) private(fromRow, toRow, fromCol, toCol, tid) reduction(max: ind)
			{
				if(object_is_found){	// if an object was found by one thread, exit parallel
					#pragma omp cancel parallel
				}
				
				tid = omp_get_thread_num();
				
				// give each thread a quarter of the matrix to work on
				if(tid < numOfThreads/2){
					fromRow = 0;  toRow = num_indexes/2;
				} else{
					fromRow = num_indexes/2;  toRow = num_indexes;
				}
				
				if(tid%2 == 0){
					fromCol = 0;  toCol = num_indexes/2;
				} else{
					fromCol = num_indexes/2;  toCol = num_indexes;
				}
					
				// search for the object in the given area
				ind = compareThreadMatrixOnGPU(dev_pic, picDim, dev_obj, objDim, match_value, dev_ind, fromRow , toRow, fromCol, toCol);
				
				if (ind != -1) {
				    object_is_found = 1;
				    #pragma omp cancel parallel
				}
			}
		}
		
		freeCudaData(dev_pic, dev_obj, dev_ind);	// after finishing the work one Picture-Object couple, free the copied memory
		
		// set the arguments for the result message. If found, send object id, picture id and position. Else, -1 for all arguments
		if(ind != -1 && picDim != 0){		
			result[0] = taskData[2];
			result[1] = ind/picDim;	// to get the row we divide by picture dimension
			result[2] = ind%picDim;	// to get the column we use the modulo of picture dimension (the returned index is in a 1d-array format)
			result[3] = taskData[0];
			
		} else {			
			result[0] = -1; result[1] = -1; result[2] = -1; result[3] = -1;
		}

		MPI_Send(result, 4, MPI_INT, 0, 0, MPI_COMM_WORLD);
		
		// free allocated memory
		free(*pictureMat);
		free(*objectMat);
	}
	return 0;
}
