/*======================================================================
 * PROJECT: Hitori Hunter
 *----------------------------------------------------------------------
 * AUTHOR: Bilal Usman
 * EMAIL: busman4@wisc.edu
 * ADDITIONAL SOURCES: NONE
 * FILE: hunter.c
 * COURSE: COMP SCI 354 - Fall 2026
 * INSTRUCTOR: Dahl
 * COPYRIGHT: 2026, Dahl
 * Posting or sharing this file with anyone outside of course staff prohibited.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "hunter.h"

// Helper for checking connectivity around a possible gate
void markReachable(Cell* solution, int size, int row, int col,
                   int blocked, int* visited);

/*
 * Entry point for this Hitori Hunter application.  This program can be used to
 * solve hitori puzzles that are encoded as plain text files.
 *
 * This program takes 1 command line arguments specifying the filename of the
 * puzzle that you would like this program to solve.
 */
int main(int argc, char** argv) {

	// use command line arguments to load board from plain text file
	if( argc != 2)
		error("Hitori Hunter usage: hunter PUZZLEFILE\n");
	char* filename = argv[1];
	int** puzzle;
	int size = loadPuzzle(filename,&puzzle);

	// transfer loaded board into a 1D array of Cells
	Cell solution[size * size];
	transferGrid(puzzle,solution,size);

	// make as much progress as possible toward solving this puzzle
	solvePuzzle(solution,size);

	// output progress toward solution to <file>.sln (length: + ".sln" + '\0')
	char solutionFilename[strlen(filename)+4+1];
	strcpy(solutionFilename,filename);
	strcat(solutionFilename,".sln");
	saveSolution(solution,size,solutionFilename);

	return 0;
}

/*
 * Helper function to more concisely report error messages through stderr
 * before ending the program with a return status of 1 (rather than 0).
 *
 * Arguments: Similar to printf, you can pass this method a format string
 * followed by a list of values to fill that string's placeholders.
 */
void error(char* format, ...) {
	va_list args;
	va_start(args,format);
	fprintf(stderr,"ERROR: ");
	vfprintf(stderr,format,args);
	exit(1);
}

/*
 * Loads Hitori puzzle from plain text file, and stores its contents into
 * a heap-allocated 2d int array.
 * 
 * The first line of this plain text file must contains a single number
 * defining the size of the square puzzle (between 1 and MAX_BOARD_SIZE).
 * Each subsequent line contains one row of comma separate numbers (as
 * many as are dedfined by the previously read size).
 */
int loadPuzzle(char* filename, int*** puzzle) {
	FILE* fin = fopen(filename,"r");
	if(fin == NULL)
		error("Unable to open file for reading: %s\n",filename);

	// reads board size
	char buffer[MAX_LINE_WIDTH];
	if(fgets(buffer,MAX_LINE_WIDTH,fin) == NULL)
		error("Unable to read first line of file: %s\n",filename);
	int size = atoi(buffer);
	if(size < 1 || size > MAX_BOARD_SIZE)
		error("Unable to read valid board size from file: %s\n",buffer);

	// allocate memory for entire puzzle grid
	int** grid = malloc(sizeof(int*)*size);
	if(grid == NULL)
		error("Unable to allocate memory for grid rows.\n");
	for(int i=0;i<size;i++) {
		grid[i] = malloc(sizeof(int)*size);
		if(grid[i] == NULL)
			error("Unable to allocate memory for grid columns.\n");
	}
	
	// read board contents
	int row = 0;
	while(fgets(buffer,MAX_LINE_WIDTH,fin) != NULL && row < size) {
		char* token;
		int col = 0;
		for(token = strtok(buffer,",");token != NULL;token = strtok(NULL,",")){
			grid[row][col] = atoi(token);
			if(grid[row][col] < 1 || grid[row][col] > MAX_BOARD_SIZE)
				error("Unable to read column number: %s\n",token);
			col++;
		}
		if(col != size) error("Not enough columns in row: %s\n",buffer);
		row++;
	}
	if(row != size) error("Not enough rows in board: %i/%i\n",row,size);

	fclose(fin);
	*puzzle = grid;
	return size;
}

/*
 * Transfers 2D heap-allocated int array into a 1D stack-allocated Cell
 * array.  The heap memory is then freed, after it has been copied..
 */
void transferGrid(int** puzzle, Cell* solution, int size) {
	// Copy every cell into the one dimensional array
	for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            solution[row * size + col] = (Cell)puzzle[row][col];
        }
    }
	// Free each row before freeing the row pointer array
	for (int row = 0; row < size; row++) {
        free(puzzle[row]);
    }
    free(puzzle);
}

/*
 * Saves solution progress to the specified file.  Circled cells are saved
 * as the circled number, blacked out cells are saved as #'s, and all other
 * cells are saved as ?'s.
 */
void saveSolution(Cell* solution, int size, char* filename) {
	FILE* fout = fopen(filename,"w");
	if(fout == NULL)
		error ("Unable to open file for writing: %s\n",filename);

	fprintf(fout,"%i\n",size);
	for(int i=0;i<size;i++) {
		for(int j=0;j<size;j++) {
			Cell value = solution[i * size + j];
			
			int isCircle = value & CIRCLE;
			int isBlackedOut = value & BLACK;
			int cellNumber = NUMBER(value);
			if(isCircle)
				fprintf(fout,"%i",cellNumber);
			else if(isBlackedOut)
				fprintf(fout,"#");
			else
				fprintf(fout,"?");
			if(j < size-1) fprintf(fout,",");
		}
		fprintf(fout,"\n");
	}
	fclose(fout);
}

void blackout(Cell* solution, int size, int row, int column) {
	int index = row * size + column;

    // Stop if this cell is already blacked out
    if (solution[index] & BLACK) {
        return;
    }

    // A circled cell cannot also be blacked out
    if (solution[index] & CIRCLE) {
        error("Cannot black out circled cell at (%d, %d).\n",
              row, column);
    }

    // Preserve the number while setting the blackout bit
    solution[index] |= BLACK;

    // Circle each neighbor that lies within the board
    if (row > 0) {
        circle(solution, size, row - 1, column);
    }
    if (row < size - 1) {
        circle(solution, size, row + 1, column);
    }
    if (column > 0) {
        circle(solution, size, row, column - 1);
    }
    if (column < size - 1) {
        circle(solution, size, row, column + 1);
    }
}

void circle(Cell* solution, int size, int row, int column) {
	int index = row * size + column;

    // Stop if this cell is already circled
    if (solution[index] & CIRCLE) {
        return;
    }

    // A blacked-out cell cannot also be circled
    if (solution[index] & BLACK) {
        error("Cannot circle blacked-out cell at (%d, %d).\n",
              row, column);
    }

    // Mark before making recursive calls to prevent repeated work
    solution[index] |= CIRCLE;
    int number = NUMBER(solution[index]);

    // Black out other matching numbers in this row
    for (int col = 0; col < size; col++) {
        int other = row * size + col;

        if (col != column &&
            !(solution[other] & BLACK) &&
            NUMBER(solution[other]) == number) {
            blackout(solution, size, row, col);
        }
    }

    // Black out other matching numbers in this column
    for (int r = 0; r < size; r++) {
        int other = r * size + column;

        if (r != row &&
            !(solution[other] & BLACK) &&
            NUMBER(solution[other]) == number) {
            blackout(solution, size, r, column);
        }
    }
}

void solvePuzzle(Cell* solution, int size) {
	makeSandwichDeductions(solution, size);
	makeDoubletDeductions(solution, size);
	makeGateDeductions(solution, size);
}

void makeSandwichDeductions(Cell* solution, int size) {
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            int index = row * size + col;

            // Equal numbers on the left and right force this cell white
            if (col > 0 && col < size - 1 &&
                NUMBER(solution[index - 1]) ==
                NUMBER(solution[index + 1])) {
                circle(solution, size, row, col);
            }

            // Equal numbers above and below force this cell white
            if (row > 0 && row < size - 1 &&
                NUMBER(solution[index - size]) ==
                NUMBER(solution[index + size])) {
                circle(solution, size, row, col);
            }
        }
    }
}

void makeDoubletDeductions(Cell* solution, int size) {
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            int index = row * size + col;
            int number = NUMBER(solution[index]);

            // A horizontal pair eliminates other matches in its row
            if (col < size - 1 &&
                NUMBER(solution[index + 1]) == number) {
                for (int otherCol = 0; otherCol < size; otherCol++) {
                    if (otherCol != col && otherCol != col + 1 &&
                        NUMBER(solution[row * size + otherCol]) == number) {
                        blackout(solution, size, row, otherCol);
                    }
                }
            }

            // A vertical pair eliminates other matches in its column
            if (row < size - 1 &&
                NUMBER(solution[index + size]) == number) {
                for (int otherRow = 0; otherRow < size; otherRow++) {
                    if (otherRow != row && otherRow != row + 1 &&
                        NUMBER(solution[otherRow * size + col]) == number) {
                        blackout(solution, size, otherRow, col);
                    }
                }
            }
        }
    }
}

void makeGateDeductions(Cell* solution, int size) {
    int cells = size * size;
    int start = -1;

    // Find a circled cell to start each connectivity search
    for (int i = 0; i < cells; i++) {
        if (solution[i] & CIRCLE) {
            start = i;
            break;
        }
    }

    if (start == -1) {
        return;
    }

    int visited[cells];
    int changed;

    do {
        changed = 0;

        for (int blocked = 0; blocked < cells; blocked++) {
            // Only test cells whose status is still undecided
            if (solution[blocked] & (BLACK | CIRCLE)) {
                continue;
            }

            // Reset the search for this candidate
            for (int i = 0; i < cells; i++) {
                visited[i] = 0;
            }

            markReachable(solution, size, start / size,
                          start % size, blocked, visited);

            // An unreachable circle means this gate must stay white
            for (int i = 0; i < cells; i++) {
                if ((solution[i] & CIRCLE) && !visited[i]) {
                    circle(solution, size,
                           blocked / size, blocked % size);
                    changed = 1;
                    break;
                }
            }
        }
    } while (changed);
}

void markReachable(Cell* solution, int size, int row, int col,
                   int blocked, int* visited) {
    // Stop before accessing a position outside the board
    if (row < 0 || row >= size || col < 0 || col >= size) {
        return;
    }

    int index = row * size + col;

    if (index == blocked || (solution[index] & BLACK) ||
        visited[index]) {
        return;
    }

    // Mark before exploring neighbors to prevent revisiting cells
    visited[index] = 1;

    markReachable(solution, size, row - 1, col, blocked, visited);
    markReachable(solution, size, row + 1, col, blocked, visited);
    markReachable(solution, size, row, col - 1, blocked, visited);
    markReachable(solution, size, row, col + 1, blocked, visited);
}

// EOF -----------------------------------------------------------------
