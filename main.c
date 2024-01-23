/*
	Parallel Computation Course - Final Project
	-------------------------------------------
	Name:	Nicole Zarch	ID:	206824310
*/

#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "cFunctions.h"

#define WORK 0
#define TERMINATE 1
#define INPUT_FILE "input.txt"
#define OUTPUT_FILE "output.txt"

int main(int argc, char *argv[]) {

	// variables for file reading
	FILE *fp;
	double match_value;
	int num_pictures;
	Picture* pictures = NULL;
	int num_objects;
	Object* objects = NULL;
	int i, j;
	
	// variables for information sending
	int taskData[4] = {0,0,0,0};	// pic id, pic dim, obj id, obj dim
	int *pictureMat;
	int *objectMat;

	// variables for process info
	int rank, nprocs;
	int inProgress = 0;
	MPI_Status status;
	
	double start, end;	// time measurement
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);	

	if(rank==0){

		start = MPI_Wtime();		
		
		fp = fopen(INPUT_FILE, "r");
		if (fp == NULL) {
			printf("Unable to open file\n");
			return -1;
		}

		printf(">> Reading File\n");
		
		// Read matching value
		fscanf(fp, "%lf", &match_value);
		printf(">> Match value is: %lf\n", match_value);
		
		// Read pictures
		readPictures(&pictures, &num_pictures, fp);
		printf(">> Read Pictures:\n");
		printPictures(&pictures, num_pictures);
		
		// Read objects
		readObjects(&objects, &num_objects, fp);
		printf(">> Read Objects:\n");
		printObjects(&objects, num_objects);
		
		fclose(fp);
		printf(">> Closed File\n");
	}

	MPI_Bcast(&match_value, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);	// broadcast the matching value to all the processes

	// Master process
	if(rank==0){	
		printf("\n--------------------------------------------\n\tWORK IN PROGRESS\n--------------------------------------------\n\n");
		int picIndex = 0;
		int objIndex = 0;
		
		// Send initial work to processes: a picture and an object to compare
		for(int p = 1; p < nprocs; p++){
			
			if(pictures[picIndex].num_found < 3) // check current picture if 3 different objects were yet to be found
				sendTask(taskData, &pictureMat, &objectMat, pictures, picIndex, objects, objIndex, p, WORK);

			inProgress++;
			objIndex++;
			if(objIndex == num_objects){
				picIndex++;
				objIndex = 0;
			}
		}

		// Collect results and send more work
		while(inProgress > 0){
		
			int object[4];    // holds: objectID, i, j, pictureID
			MPI_Recv(object, 4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			
			if(object[3] != -1){	// if picture id field is not -1, it means that the object was foun in the picture
				updatePicture(pictures, num_pictures, object[0], object[1], object[2], object[3]);
			}
			
			inProgress--;

			if(picIndex < num_pictures){ // if more work left
				
				if(pictures[picIndex].num_found < 3) // check current picture if 3 different objects were yet to be found
					sendTask(taskData, &pictureMat, &objectMat, pictures, picIndex, objects, objIndex, status.MPI_SOURCE, WORK);

				inProgress++;
				objIndex++;
				if(objIndex == num_objects){
					picIndex++;
					objIndex = 0;
				}

			} else {    // no work left - terminate process
			
				MPI_Send(taskData, 4, MPI_INT, status.MPI_SOURCE, TERMINATE, MPI_COMM_WORLD);
			}

		}

		end = MPI_Wtime();	
				
		printf("---- Elapsed time is %f ----\n", end-start);
		printPicturesDetails(pictures, num_pictures, stdout);
		printPicturesDetailsToFile(pictures, num_pictures, OUTPUT_FILE);
		freeResources(&pictures, num_pictures, &objects, num_objects);

	}
	
	// Worker process
	else {
		
		int tag;
		int result[4];
		do{
			receiveTask(taskData, &pictureMat, &objectMat, match_value, result, &status, &tag);

		}while(tag == WORK);   	 
	}
	
	MPI_Finalize();
	return 0;
}

