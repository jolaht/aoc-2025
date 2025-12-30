/* The input is always a square. Let N denote its side length.
 * We define a procedure sweep(bool dry), which counts the number of
 * removable tiles. If dry is true, the prodecure "dry-runs", and does not
 * mutate input (P1). Otherwise each counted tile is also removed as soon as it
 * is seen. */

#include <stdio.h>
typedef unsigned int ui;

char input[] = {
#embed "input.txt"
};
#define N 135

/*
char input[] = {
#embed "example.txt"
};
#define N 10
*/

/* Access the input by a given zero-indexed row and column. */
char *rc(ui row, ui col) { return input + (N+1)*row + col; }
bool is_on_grid(ui row, ui col) { return row < N && col < N; }
ui count_adjacents(ui row, ui col) {
	ui count = 0;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if (i == 0 && j == 0) continue;
			ui r = row+i, c = col+j;
			count += is_on_grid(r, c) && *rc(r, c) == '@';
		}
	}
	return count;
}
ui sweep(bool dry) {
	ui count = 0;
	for (ui r = 0; r<N; r++) {
		for (ui c = 0; c<N; c++) {
			char *x = rc(r, c);
			if (*x == '@' && count_adjacents(r, c) < 4) {
				count++;
				if (!dry)
					*x = '.';
			}
		}
	}
	return count;
}
ui p1(void) { return sweep(true); }
ui p2(void) {
	ui count = 0, d = 0;
	do { d = sweep(false); count += d; } while (d);
	return count;
}

int main(void) {
	printf("p1: %u\n", p1());
	printf("p2: %u\n", p2());
	return 0;
}
