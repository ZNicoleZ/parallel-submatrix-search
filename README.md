# Parallel Implementation of a Submatrix Search 🔍
An object detection algorithm using parallel computation techniques in a Linux environment and C language.

## Problem Definition  
- Pictures (N) and Objects (N): Square matrices of integers with N rows and N columns, representing sets of Pictures and Objects of different sizes.
- Each matrix member represents a "color" within the range of possible colors [1, 100].
- Position (I, J) defines the coordinates of the upper-left corner of the Object in the Picture.
- For each pair of overlapping members p and o of the Picture and Object, a relative difference will be calculated.

This project determines whether a given picture contains at least three objects from the given object set.

## Rationale and Architecture:  
The main idea behind my project is to dynamically assign tasks to different processes. A "task" is a search for one object in one picture (if there are yet to be found three different objects in that picture). For example, if there are N pictures and M objects in the input file, we will end up with NxM tasks at most. 
With this approach, I aimed to address load balancing and ensure that a specific process won't constantly work on a large amount of information (e.g., one process will consistently work on a 900x900 matrix while others will work on 50x50 matrices).

I implemented the parallel approach with the following components:

- **MPI:** I used MPI to send the picture-object couple data (a task) to the working process. The working processes return the result to the master process:
  - If the object was found in the picture, the master will receive the object ID and position.
  - If it wasn't found, it will receive default values.

- **OpenMP:** I divided the processing of the picture's submatrix of possible positions into different threads (possible values for i and j are between 0 and pictureDimension-objectDimension). Each thread will work on its part of the matrix and look for the object with a CUDA kernel function in its given range. Once the object is found by one thread, the others will stop and exit the parallel section.

- **CUDA:** I implemented a kernel function that will be called by threads in the OpenMP parallel section and use the GPU's threads to try and locate the object in the valid positions within the passed range. Each GPU thread will be responsible for checking a specific position of the overlapping object on the picture.

## Results:  
96% reduction in processing time. The average processing time decreased from 59 seconds (serial solution) to just 2.3 seconds on two machines.

