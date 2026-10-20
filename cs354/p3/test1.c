#include <stdio.h>
#include "gator.h"

int main(int argc, char** argv) {

	ag_init();

	void *p = ag_alloc(20);
	printf("Payload: %p\n", p);
	
	ag_visualize();

	void *q = ag_alloc(4044);
	printf("Second payload: %p\n", q);
	ag_visualize();
	printf("When full: %p\n", ag_alloc(1));

	printf("Free p: %d\n", ag_free(p));
	ag_visualize();
	printf("Free p again: %d\n", ag_free(p));
	printf("Free NULL: %d\n", ag_free(NULL));

	printf("Free q: %d\n", ag_free(q));
	ag_visualize();

	void *a = ag_alloc(20);
	void *b = ag_alloc(20);
	void *c = ag_alloc(20);

	ag_free(b);
	ag_free(a);
	printf("Merged with next:\n");
	ag_visualize();

	ag_free(c);
	printf("Merged with both:\n");
	ag_visualize();

	void *large = ag_alloc(60);
	ag_alloc(12);  // Keep the free holes separated
	void *small = ag_alloc(44);
	ag_alloc(12);  // Keep the smaller hole separate from the remaining heap

	ag_free(large);
	ag_free(small);

	void *chosen = ag_alloc(20);
	printf("Best fit chose smaller hole: %d\n", chosen == small);
	ag_visualize();
	
	return 0;
}
