
// Picture struct definition
typedef struct {
	int id;
	int dimension;
	int *matrix;
	int num_found;  	  	// number of found objects
	int objects_found[3][3];	// 3 columns: object ID, position i, position j
} Picture;

// Object struct definition
typedef struct {
	int id;
	int dimension;
	int *matrix;
} Object;
