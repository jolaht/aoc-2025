#include <stdio.h>
#include <stdlib.h>
typedef unsigned long long ull;
typedef unsigned short us;
#define $PANIC(lit) do { puts("Panic: " lit); exit(0); } while (0)
#define $EXPECT(cond, lit) do { if (!(cond)) $PANIC(lit); } while (0)

/* -- Introduction --
 * The solution can be brute forced in O(n^k), where n is the length of a bank
 * and k is the amount of batteries to select. In P1, k=2, but in P2, k=12.
 * Brute force is not the optimal way.
 *
 * For P1 I initially considered using the properties of gaps between the
 * numbers. If you know the largest number before a gap, computing the largest
 * number in the following gap is O(1). Same for the largest number after the
 * gap. This gives two lists of length n, which are O(n) to compute. The
 * solution is then the maximum value of the concatenated pairwise zip of these
 * arrays. This runs in O(n) for P1, but for P2 I did not proceed with this. */

/* -- Greedy algorithm --
 * We are asked to form the largest n-digit number by selecting digits from a
 * given number. I got the correct answer by hypothetising that the problem has
 * the greedy choice property that the numbers can be chosen one by one from
 * the most significant to the least significant, by considering the subinput
 * with the tail of n-1 digits removed, and selecting the rightmost largest
 * digit from that, and recursing.
 * Example. Choosing the largest 3-digit number from "81811112111".
 * The first digit is the largest rightmost number in "818111121" (tail of
 * 2 digits removed), which is the first 8.
 * Now recurse, and choose the largest 2-digit number from "181111211" (tail of
 * 1 removed), which is 8. Recurse by choosing the largest number from
 * "11112111", which is 2. We get: 882. This runs in O(n). */

char input[] = {
#embed "input.txt"
};
#define N_BATTERIES 100
#define N_BANKS 200

/*
char input[] = {
#embed "example.txt"
};
#define N_BATTERIES 15
#define N_BANKS 4
*/

struct Bank { us joltages[N_BATTERIES]; };
struct Banks { struct Bank arr[N_BANKS]; };
void parse(struct Banks *banks) {
	char *c = input;
	for (us i=0; i<N_BANKS; i++) {
		for (us j=0; j<N_BATTERIES; j++)
			banks->arr[i].joltages[j] = (*c++ - '0');
		if (*c == '\n')
			c++;
	}
}

/* Recursively solves the problem for k>0 by taking the locally optimal
 * decision each time. */
ull _f(ull acc, struct Bank *b, us start, us k) {
	if (k == 0) return acc;
	$EXPECT(N_BATTERIES-start+1 >= k, "sequence not possible");
	/* Takes the leftmost highest value in the subsequence that still leaves k-1
	 * numbers unused for the rest of the sequence. */
	us best = start;
	for (us i = start+1; i<N_BATTERIES-k+1; i++) {
		if (b->joltages[i] > b->joltages[best])
			best = i;
	}
	return _f(10 * acc + b->joltages[best], b, best+1, k-1);
}
ull f(struct Bank *b, us k) { return _f(0, b, 0, k); }

struct Solution { ull p1, p2; };
struct Solution solve_p1(struct Banks *banks) {
	ull p1 = 0, p2 = 0;
	for (us i=0; i<N_BANKS; i++) {
		p1 += f(&banks->arr[i], 2);
		p2 += f(&banks->arr[i], 12);
	}
	return (struct Solution) {p1, p2};
}

int main(void) {
	struct Banks banks = {0};
	parse(&banks);
	struct Solution s = solve_p1(&banks);
	printf("p1: %llu\np2: %llu\n", s.p1, s.p2);
}
