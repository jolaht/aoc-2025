#include <stdio.h>
typedef unsigned int ui;
typedef unsigned long long ull;

/* The puzzle has the property that each invalid case can be found by
 * the total area of the pieces not fitting within the grid. */

/*
char input[] = {
#embed "example.txt"
};
#define N_REGIONS 3
*/
char input[] = {
#embed "input.txt"
};
#define N_REGIONS 1000

#define N_SHAPES 6
struct Shape { ui area; };
struct Shapes { struct Shape arr[N_SHAPES]; ull l; };
struct Region { ui x, y, shape_amts[N_SHAPES]; };
struct Regions { struct Region arr[N_REGIONS]; ull l; }; 

#define $IS_NUMERIC(c) ('0' <= c && c <= '9')
/* Scans a one or two digit uint from c. */
ui scanui(char *c) {
	ui r = 0;
	while ($IS_NUMERIC(*c))
		r = r*10 + (*c++ - '0');
	return r;
}
#define $CONSUME_WHILE(cond) do { while(cond) c++; } while (0)
#define $CONSUME_NUMERIC $CONSUME_WHILE($IS_NUMERIC(*c))
#define $CONSUME(x) $CONSUME_WHILE(*c == x)
void parse(struct Shapes *shapes, struct Regions *regs) {
	shapes->l = 0; regs->l = 0;
	char *c = input;
	for (ui i=0; i<N_SHAPES; i++) {
		$CONSUME_NUMERIC; $CONSUME(':'); $CONSUME('\n');
		for (ui j=0; j<3; j++) {
			for (ui k=0; k<3; k++)
				shapes->arr[i].area += *c++ == '#';
			$CONSUME('\n');
		}
	}

	for (ui i=0; i<N_REGIONS; i++) {
		regs->arr[i].x = scanui(c); $CONSUME_NUMERIC; $CONSUME('x');
		regs->arr[i].y = scanui(c); $CONSUME_NUMERIC;
		$CONSUME(':'); $CONSUME(' ');
		for (ui j=0; j<N_SHAPES; j++) {
			regs->arr[i].shape_amts[j] = scanui(c); $CONSUME_NUMERIC;
			$CONSUME(' ');
		}
		$CONSUME('\n');
	}
}

ui p1(struct Shapes *shapes, struct Regions *regions) {
	/* Count of regions which are impossible to fill based on total area. */
	ui count = 0;
	for (ui i=0; i<N_REGIONS; i++) {
		const struct Region *r = &regions->arr[i];
		ui area = r->x*r->y;
		ui required_area = 0;
		for (ui j=0; j<N_SHAPES; j++) {
			const struct Shape *s = &shapes->arr[j];
			required_area += s->area * r->shape_amts[j];
		}
		count += area < required_area;
	}
	return N_REGIONS - count;
}

int main(void) {
	struct Shapes shapes = {0};
	struct Regions regions = {0};
	parse(&shapes, &regions);
	printf("p1: %u\n", p1(&shapes, &regions));
	return 0;
}
