#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define $PANIC(msg) do { puts("Panic: " msg); exit(1); } while (0)
#define $EXPECT(cond, msg) do {if (!(cond)) $PANIC(msg); } while(0)
typedef unsigned long long ull;
typedef unsigned int ui;

char input[] = {
#embed "input.txt"
, '\0'
};
#define N_RANGES 37

struct Ranges { ull his[N_RANGES], los[N_RANGES]; };
#define $CONSUME_ONCE(x) do { if (*c == x) c++; } while (0)
void parse(struct Ranges *ranges) {
	char *c = input; int delta;
	for (ui i=0; i<N_RANGES; i++) {
		sscanf(c, "%llu-%llu%n", &ranges->los[i], &ranges->his[i], &delta);
		c += delta; $CONSUME_ONCE(',');
	}
}

/* -- P1 --
 * L1: Any invalid ID x must have an even length (in its base 10
 * representation), n*2, where n in Z.
 * L2: An ID is invalid, if its length is even, and if for each i
 * in [0 until n]: str[i]=str[i+n].
 *
 * Let valid: id -> boolean
 * Let p = 10^n.
 * Therefore, valid(x) iff. x/p = x%p and the length is even */


ull powers[] = {
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
	1000000000, 10000000000, 100000000000, 1000000000000, 10000000000000,
	100000000000000, 1000000000000000, 10000000000000000,
	100000000000000000, 1000000000000000000,
};
#define N_POWS (sizeof(powers)/sizeof(powers[0]))
/* Gives the amount of digits (in base-10) of a number.
 * This can be thought of as floor(log10(x))+1 */
unsigned int digits(ull x) {
	for (int i=0; i<N_POWS; i++) {
		if (x<powers[i])
			return i;
	}
	return N_POWS;
}
// Returns n**10
ull pow10(ull n) {
	$EXPECT(n < N_POWS, "n too large in pow10");
	return powers[n];
}

ull p1(struct Ranges *ranges) {
	ull sum = 0;
	for (ui i=0; i<N_RANGES; i++) {
		ull lo = ranges->los[i], hi = ranges->his[i];
		for (ull x = lo; x<=hi; x++) {
			unsigned int d = digits(x); ull p = pow10(d/2);
			if (d % 2 == 0 && x/p == x%p)
				sum += x;
		}
	}
	return sum;
}

/* -- P2 --
 * Let d be the length of the number in digits.
 * If the number is made of a sequence of length m repeating: m must divide d.
 *
 * L1: x is an invalid ID iff:
 * For any m in [1 to floor(d/2)] that divides d:
 * The sequence of m digits from the start repeats d/m times.
 *
 * Let 'k:th m-sequence' refer to the m-digit long potentially repeating part,
 * with index k in [1 to d/m]
 * Let the potentially repeating sequence of m digits be the m-sequence.
 *
 * L2: The k:th m-sequence of x = x/10^(d-k*m) mod 10^m.
 * This was constructed from an example listed below. */

/* Example of L2:
 * Let x = 123456
 * 1st 2-sequence: x/10^4 mod 10^2 = 12 mod 100 = 12
 * 2nd 2-sequence: x/10^2 mod 10^2 = 1234 mod 100 = 34
 * 3rd 2-sequence: x/10^0 mod 10^2 = 123456 mod 100 = 56
 *
 * Alternative explanation:
 * x/10^k is a base10-right-k-shift.
 * x mod 10^k keeps the first k digits while zeroing others */

ull p2(struct Ranges *ranges) {
	ull sum = 0;
	for (ui i=0; i<N_RANGES; i++) {
		ull lo = ranges->los[i], hi = ranges->his[i];

		for (ull x = lo; x<=hi; x++) {
			unsigned int d = digits(x);
		
			bool is_invalid = 0;
			// Consider each m-sequence where m divides d
			for (unsigned int m = 1; m<=d/2; m++) {
				if (d%m == 0) {
					ull head = x / pow10(d-m); // The first m-sequence
					// x is an invalid id iff. for all k in [2 until d/m]:
					// k:th m-sequence == head
					bool b = true; // Represents whether all m-sequences equal
					for (unsigned int k = 2; k<=d/m; k++) {
						ull part = x/pow10(d-k*m) % pow10(m);
						b &= head == part;
					}
					is_invalid |= b;
				}
			}
			if (is_invalid)
				sum += x;
		}
	}
	return sum;
}

int main(void) {
	struct Ranges ranges = {0};
	parse(&ranges);
	printf("p1: %llu\n", p1(&ranges));
	printf("p2: %llu\n", p2(&ranges));
	return 0;
}
