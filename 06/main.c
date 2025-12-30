#include <stdio.h>
#include <stdlib.h>

typedef unsigned int ui;
typedef unsigned long long ull;
#define $PANIC(lit) do { puts("Panic: " lit); exit(1); } while (0)
#define $EXPECT(cond, lit) do { if (!(cond)) $PANIC(lit); } while (0)

bool is_op(char c) { return c == '+' || c == '*'; }

/* N_ROWS is the number of numeric rows.
 * The row with index N_ROWS contains the operations.
 * N_COLUMNS is the length of each line (including space, excluding newline). */

/*
char input[] = {
#embed "example.txt"
};
#define N_ROWS 3
#define N_COLUMNS 15
*/
char input[] = {
#embed "input.txt"
};
#define N_ROWS 4
#define N_COLUMNS 3724

char *table(ui row, ui col) { return input + row*(N_COLUMNS+1) + col; }

/* A column is considered a break if it contains only blanks
 * or if it is located past the end of the table. */
bool is_break(ui col) {
	if (col >= N_COLUMNS)
		return true;
	bool r = true;
	for (ui row = 0; row<N_ROWS; row++)
		r &= *table(row, col) == ' ';
	return r;
}

#define $CONSUME(x) do { while (*c == x) c++; } while (0)
ull p1(void) {
	ull sum = 0; ui col = 0;
	/* This outer loop expects a state where col points to the start of a block.
	 * This column is expected to contain an operator. */
	while (col < N_COLUMNS) {
		char op = *table(N_ROWS, col);
		$EXPECT(op == '+' || op == '*', "p1 expected an operator");
		ull acc = (op == '*');
		for (ui row=0; row<N_ROWS; row++) {
			char *c = table(row, col);
			$CONSUME(' ');
			ull n = 0;
			while ('0' <= *c && *c <= '9')
				n = n*10 + (*c++ - '0');
			if (op == '+')
				acc += n;
			if (op == '*')
				acc *= n;
		}
		while (!is_break(col))
			col++;
		col++;
		sum += acc;
	}
	return sum;
}

#define C_BLOCK 5
ull p2(void) {
	ull block[C_BLOCK] = {0}; ui block_l = 0;
	char op = 0;
	ull sum = 0;
	for (ui col = 0; col<=N_COLUMNS; col++) {
		if (is_break(col)) {
			ull acc = (op == '*');
			for (ui i = 0; i<block_l; i++) {
				if (op == '+')
					acc += block[i];
				if (op == '*')
					acc *= block[i];
			}
			sum += acc;
			block_l = 0; op = 0;
		} else {
			char maybe_op = *table(N_ROWS, col);
			if (is_op(maybe_op))
				op = maybe_op;
			ull n = 0;
			for (ui row = 0; row<N_ROWS; row++) {
				char c = *table(row, col);
				if ('0' <= c && c <= '9')
					n = n*10 + (c-'0');
			}
			block[block_l++] = n;
		}
	}
	return sum;
}

int main(void) {
	printf("p1: %llu\n", p1());
	printf("p2: %llu\n", p2());
	return 0;
}
