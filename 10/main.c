#include <stdio.h>
#include <stdlib.h>
#include <stdbit.h>
#include <limits.h>
typedef unsigned long long ull;
typedef unsigned int ui;
typedef unsigned short us;
#define $PANIC(lit) do { puts("Panic: " lit); exit(1); } while (0)
#define $EXPECT(cond, lit) do {if (!(cond)) $PANIC(lit); } while (0)

/*
char input[] = {
#embed "example.txt"
};
#define N_MACHINES 3
*/
char input[] = {
#embed "input.txt"
};
#define N_MACHINES 167

#define C_BUTTONS 13
#define C_OUTS 10

/* -- Input repr (P1) --
 * Satisfying states for lights (output) and the input wirings essentially
 * translate to boolean arrays. However, the amount of lights is always <= 10,
 * which is small enough to use u16s. This also simplifies applying an input to
 * a bitwise xor.
 * 'outs' contains the bit representation of the satisfying end state. The nth
 * light has the satisfying state ((outs >> n) & 1). Buttons are stored such
 * that they are applied with XORing the current state. */

struct Machine {
	us outs, joltages[C_OUTS], n_outs, buttons[C_BUTTONS], n_buttons;
};
struct Machines { struct Machine arr[N_MACHINES]; };

#define $CONSUME(x) do {while (*c == x) c++;} while (0)
#define $CONSUME_UNTIL(x) do {while (*c != x) c++;} while (0)
void parse(struct Machines *machines) {
	char *c = input;
	for (ull i=0; i<N_MACHINES; i++) {
		struct Machine *mach = &machines->arr[i];
		mach->n_outs = 0; mach->n_buttons = 0;
		us sat = 0; $CONSUME('[');
		while (*c != ']') {
			sat |= (*c == '#') << mach->n_outs;
			mach->n_outs++; c++;
		} $CONSUME(']'); $CONSUME(' ');
		mach->outs = sat;
		
		while (*c != '{') {
			$CONSUME('(');
			ull button = 0;
			while (*c != ')') {
				$EXPECT('0' <= *c && *c <= '9', "expected a number");
				us m = *c - '0'; button |= 1 << m;
				c++; $CONSUME(',');
			}
			$EXPECT(mach->n_buttons < C_BUTTONS, "button capacity exceeded");
			mach->buttons[mach->n_buttons++] = button;
			$CONSUME(')'); $CONSUME(' ');
		} $CONSUME('{');

		us n_joltages = 0;
		while (*c != '}') {
			us joltage; int d; sscanf(c, "%hu%n", &joltage, &d); c+= d;
			$EXPECT(joltage < (2<<8), "joltage too high");
			mach->joltages[n_joltages++] = joltage; $CONSUME(',');
		} $CONSUME('}');
		$CONSUME('\n');
	}
}

#define $MIN(a, b) (a < b ? a : b)
ull p1(const struct Machines *machs) {
	ull sum = 0;
	for (ull i=0; i<N_MACHINES; i++) {
		const struct Machine *m = &machs->arr[i];
		/* Smallest count of buttons found st. machine is satisfied */
		us best = USHRT_MAX;
		/* Loop through every sequence j of n_buttons bits.
		 * The bits of j determine which buttons to toggle.
		 * (Any button is only either toggled or not) */
		for (ull j=0; j<(1<<m->n_buttons); j++) {
			us state = 0;
			for (ull k=0; k<m->n_buttons; k++) {
				if (j >> k & 1)
					state ^= m->buttons[k];
			}
			if (state == m->outs)
				best = $MIN(best, stdc_count_ones(j));
		}
		if (best != USHRT_MAX)
			sum += best;
	}
	return sum;
}

bool all_are_zero(const us joltages[C_OUTS]) {
	bool r = true;
	for (us i=0; i<C_OUTS; i++)
		r &= joltages[i] == 0;
	return r;
}

/* -- Pattern cost precomputation --
 * Output parity -> (Joltage -> Cost): B^C_OUTS -> (J^C_OUTS -> C)
 * Capacity of Joltage->Cost -mappings is 2^C_BUTTONS = 8192.
 * Best to have a pool-allocated LL, with an array to map output parity
 * to LL-heads? */
struct JoltageCostLLN {
	us joltages[C_OUTS]; ull cost;
	struct JoltageCostLLN *next;
};
/* Amount of possible values for output parity. */
#define C_PARITY (1 << C_OUTS)
#define C_JOLTAGES (1 << C_BUTTONS)
struct ParityJoltageCostMap {
	struct JoltageCostLLN *heads[C_PARITY];
	struct JoltageCostLLN pool[C_JOLTAGES]; size_t pool_l;
};
/* Allocates one LLN, appending it to the end of the LL indexed by 'parity'.
 * This ended up being unused as I decided to check for duplicates as well
 * for a ~10% reduction in runtime.*/
struct JoltageCostLLN *pjcm_lln_append(
	struct ParityJoltageCostMap *map, us parity
) {
	struct JoltageCostLLN **p = &map->heads[parity];
	while (*p)
		p = &(**p).next;
	*p = &map->pool[map->pool_l++];
	return *p;
}
struct JoltagePacking { ull a, b; };
/* Used to make comparing joltages faster.
 * Relies on each joltage being under 2^8 (fitting in 8 bits). */
struct JoltagePacking j_pack(us joltages[C_OUTS]) {
	ull a = 0, b = 0;
	a |= joltages[0] | (joltages[1] << 8) | (joltages[2] << 16);
	a |= (joltages[3] << 24) | ((ull) joltages[4] << 32);
	b |= joltages[5] | (joltages[6] << 8) | (joltages[7] << 16);
	b |= (joltages[8] << 24) | ((ull) joltages[9] << 32);
	return (struct JoltagePacking) { .a = a, .b = b };
}
bool j_pack_eq(struct JoltagePacking a, struct JoltagePacking b) {
	return a.a == b.a && a.b == b.b;
}

/* Expects a zeroed map. */
void pjcm_generate(struct ParityJoltageCostMap *map, struct Machine *m) {
	/* 'j' is the bit-array indicating button states (bool),
	 * cycling through all possibilities. */
	for (ull j=0; j<(1<<m->n_buttons); j++) {
		us joltages[C_OUTS] = {0};
		us parity = 0;
		for (us k=0; k<m->n_buttons; k++) {
			if (j>>k & 1) {
				us b = m->buttons[k];
				parity ^= b;
				for (us i=0; i<m->n_outs; i++)
					joltages[i] += b>>i & 1;
			}
		}

		struct JoltageCostLLN **llnp = &map->heads[parity];
		struct JoltagePacking pack1 = j_pack(joltages);
		bool joltage_found = false;
		while (*llnp && !joltage_found) {
			if (j_pack_eq(pack1, j_pack((*llnp)->joltages))) {
				joltage_found = true;
				break;
			}
			llnp = &(*llnp)->next;
		}
		ull cost = stdc_count_ones_us(j);
		if (joltage_found) {
			struct JoltageCostLLN *lln = *llnp;
			if (cost < lln->cost) {
				for (us i=0; i<C_OUTS; i++)
					lln->joltages[i] = joltages[i];
				lln->cost = cost;
			}
		} else {
			// LL-append
			*llnp = &map->pool[map->pool_l++];
			struct JoltageCostLLN *lln = *llnp;
			for (us i=0; i<C_OUTS; i++)
				lln->joltages[i] = joltages[i];
			lln->cost = cost; lln->next = NULL;
		}

	}
}

/* P2, single machine solver.
 * Returns the least amount of button presses required to zero the given
 * joltages with the buttons of the given machine.
 * If a machine is unsolvable for given joltages, returns (ull) -1.*/
#define DEFOPT (1<<16)
ull _solve_machine(
	struct Machine *m, struct ParityJoltageCostMap *pjcm, us joltages[C_OUTS]
) {
	if (all_are_zero(joltages))
		return 0;
	us parity = 0; ull optimum = DEFOPT;
	for (us i=0; i<C_OUTS; i++)
		parity |= (joltages[i] % 2) << i;
	for (struct JoltageCostLLN *lln = pjcm->heads[parity]; lln; lln=lln->next) {
		bool issok = true;
		for (us i=0; i<C_OUTS; i++)
			issok &= lln->joltages[i] <= joltages[i];
		if (issok) {
			us reduced[C_OUTS];
			for (us i=0; i<C_OUTS; i++)
				reduced[i] = (joltages[i] - lln->joltages[i]) / 2;
			ull cost = lln->cost + 2 * _solve_machine(m, pjcm, reduced);
			optimum = $MIN(optimum, cost);
		}
	}
	return optimum;
}
ull solve_machine(struct Machine *m) {
	struct ParityJoltageCostMap pjcm = {0};
	pjcm_generate(&pjcm, m);
	ull solution = _solve_machine(m, &pjcm, m->joltages);
	$EXPECT(solution != -1, "no solution found");
	return solution;
}
ull p2(struct Machines *machines) {
	ull sum = 0;
	for (ui i=0; i<N_MACHINES; i++) {
		ull sol = solve_machine(&machines->arr[i]);
		// printf("Machine %u/%u: %llu\n", i+1, N_MACHINES, sol);
		sum += sol;
	}
	return sum;
}

int main(void) {
	struct Machines machines = {0};
	parse(&machines);
	printf("p1: %llu\n", p1(&machines));
	printf("p2: %llu\n", p2(&machines));
	return 0;
}
