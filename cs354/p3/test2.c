#include <stdio.h>
#include "gator.h"

#define HEAP_OPS 1000
#define ARRAY_SIZE 100
#define BLOCK_SIZES 8,12,32

int main(int argc, char** argv) {

	if(argc != 2) {
		fprintf(stderr,"Usage: gator_test SEED");
		return 1;
	}
	int seed = atoi(argv[1]);
	srand(seed);

	int block_sizes[] = { BLOCK_SIZES };
	int block_size_count = sizeof(block_sizes)/sizeof(int);
	void* pointers[ARRAY_SIZE] = {0};	
	for(int i=0;i<HEAP_OPS;i++) {
		int index = (rand() & 0x7fffffff) % ARRAY_SIZE;
		if(pointers[index] != NULL) {
			ag_free(pointers[index]);
			pointers[index] = NULL;
		} else {
			int size_index = (rand() & 0x7fffffff) % block_size_count;
			pointers[index] = ag_alloc(block_sizes[size_index]);
		}
	}

	printf("AFTER %d HEAP OPS: \n",HEAP_OPS);
	ag_visualize();

	for(int i=0;i<ARRAY_SIZE;i++)
		if(pointers[i] != NULL)
			ag_free(pointers[i]);

	printf("AFTER FREEING EVERYTHING.\n");
	ag_visualize();
	
	return 0;
}
