/*======================================================================
 * FILE: hunter.h
 * PROJECT: Hitori Hunter
 * COURSE: COMP SCI 354 - Fall 2026
 * INSTRUCTOR: Dahl
 * COPYRIGHT: 2026, Dahl
 * Posting or sharing this file with anyone outside of course staff prohibited.
 *----------------------------------------------------------------------
 * DO NOT MAKE ANY CHANGES TO THIS FILE.
 *----------------------------------------------------------------------
 */

#ifndef _HUNTER_H_
#define _HUNTER_H_

#define MAX_BOARD_SIZE 32
#define MAX_LINE_WIDTH 80

typedef unsigned char Cell;
#define BLACK 0x80
#define CIRCLE 0x40
#define NUMBER(X) (X & 0x1F)

int loadPuzzle(char* filename, int*** puzzle);
void saveSolution(Cell* solution, int size, char* filename);

void transferGrid(int** puzzle, Cell* solution, int size);

void blackout(Cell* solution, int sie, int row, int column);
void circle(Cell* solution, int size, int row, int column);

void solvePuzzle(Cell* solution, int size);
void makeSandwichDeductions(Cell* solution, int size);
void makeDoubletDeductions(Cell* solution, int size);

void makeGateDeductions(Cell* solution, int size); // optional: extra credit

void error(char* format, ...);

#endif // _HUNTER_H_
