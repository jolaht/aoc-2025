#include <stdio.h>
#include <stdlib.h>
typedef unsigned int ui;
typedef unsigned long long ull;
#define $PANIC(lit) do { puts("Panic: " lit); exit(1); } while (0)
#define $EXPECT(cond, lit) do {if (!(cond)) $PANIC(lit); } while (0)

/*
char input[] = {
#embed "example.txt"
};
#define N_COLS 15
*/

char input[] = {
#embed "input.txt"
};
#define N_COLS 141

#define N_ROWS (N_COLS + 1)
char *table(ui row, ui col) { return input + row*(N_COLS+1)+col; }
ull p1() {
	ull count = 0;
	for (ui row=1; row<N_ROWS; row++) {
		for (ui col=0; col<N_COLS; col++) {
			char above = *table(row-1, col);
			char *this = table(row, col);
			count += (*this == '^' && above == '|');
			/* After this point, we only handle beam propagation. */
			if (*this != '.')
				continue;
			/* Propagate a beam in empty space*/
			if (above == '|' || above == 'S')
				*table(row, col) = '|';
			/* Propagate a beam from the left of a splitter */
			if (col != 0) {
				char left = *table(row, col-1);
				char left_above = *table(row-1, col-1);
				if (left == '^' && left_above == '|')
					*this = '|';
			}
			/* Propagate a beam from the right of a splitter */
			if (col != N_COLS-1) {
				char right = *table(row, col+1);
				char right_above = *table(row-1, col+1);
				if (right == '^' && right_above == '|')
					*this = '|';
			}
		}
	}
	return count;
}

/* P2. Iterate rows from the bottom. For each row, store the number
 * of ways to get to that column. All initialised to 1. If a splitter is
 * encountered for col, then the number of ways for that column is discarded,
 * and made equal to the sum of the ways of getting to its sides. */
#define NCOLS 200

ull p2() {
	ull ways[N_COLS];
	for (ui col=0; col<N_COLS; col++)
		ways[col] = 1;
	// Loop throgh [0 until nrows].reverse
	for (ui row=N_ROWS; row-- > 0; ) {
		/* Loop through columns excluding the edges. No splitter can exist on
		 * the edges, and S does not exist on those either. */
		for (ui col=1; col<N_COLS-1; col++) {
			char this = *table(row, col);
			if (this == '^')
				ways[col] = ways[col-1] + ways[col+1];
			if (this == 'S')
				return ways[col];
		}
	}
	return 0;
}

int main(void) {
	printf("p1: %llu\n", p1());
	printf("p2: %llu\n", p2());
	return 0;
}
