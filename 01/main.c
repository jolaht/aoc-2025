#include <stdio.h>

char input[] = {
#embed "input.txt"
};

struct Solution { int p1, p2; };
struct Solution solve(void) {
	char *c = input; int delta;
	char d; int m;

	int n_p1 = 50, n_p2 = 50;
	int r_p1 = 0, r_p2 = 0;
	while (sscanf(c, "%c%i\n%n", &d, &m, &delta) == 2) {
		c += delta;

		n_p1 += (d == 'R' ? m : -m);
		r_p1 += (n_p1 % 100 == 0);

		for (int i = 0; i<m; i++) {
			n_p2 += (d == 'R') - (d == 'L');
			r_p2 += (n_p2 % 100 == 0);
		}
	}
	return (struct Solution) { .p1 = r_p1, .p2 = r_p2 };
}

int main(void) {
	struct Solution s = solve();
	printf("p1: %i\np2: %i\n", s.p1, s.p2);
	return 0;
}
