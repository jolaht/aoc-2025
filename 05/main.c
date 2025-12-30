#include <stdio.h>
#include <stdlib.h>
typedef unsigned long long ull;
typedef unsigned int ui;
#define $PANIC(lit) do { puts("Panic: " lit); exit(1); } while (0)
#define $EXPECT(cond, lit) do { if(!(cond)) $PANIC(lit); } while (0)

char input[] = {
#embed "input.txt"
};
#define N_RANGES 173
#define N_INGREDIENTS 1000

/*
char input[] = {
#embed "example.txt"
};
#define N_RANGES 4
#define N_INGREDIENTS 6
*/

struct Database {
	ull fresh_los[N_RANGES], fresh_his[N_RANGES], ingredients[N_INGREDIENTS];
};

void parse(struct Database *db) {
	char *c = input; int d;
	ull lo, hi;
	for (ui i=0; i<N_RANGES; i++) {
		int r = sscanf(c, "%llu-%llu%n", &lo, &hi, &d);
		c += d; $EXPECT(r == 2, "failed to parse ranges");
		db->fresh_los[i] = lo; db->fresh_his[i] = hi;
	}
	ull id;
	for (ui i=0; i<N_INGREDIENTS; i++) {
		int r = sscanf(c, "%llu%n", &id, &d);
		c += d; $EXPECT(r == 1, "failed to parse ingredients");
		db->ingredients[i] = id;
	}
}

bool is_fresh(struct Database *db, ull x) {
	for (size_t i = 0; i<N_RANGES; i++) {
		if (db->fresh_los[i] <= x && x <= db->fresh_his[i])
			return true;
	}
	return false;
}
ull p1(struct Database *db) {
	uint count = 0;
	for (size_t i = 0; i<N_INGREDIENTS; i++)
		count += is_fresh(db, db->ingredients[i]);
	return count;
}

/* -- P2 --
 * The amount of items contained within the ranges is very large, enumerating
 * them is not an option. Let's make a data structure to represent a list of
 * ranges, such that the ranges internally are not allowed to overlap. Any
 * insertion of a range mutates the data structure such that no ranges overlap
 * after the insertion, assuming the state before was valid. */

// RangeSet rs
// rs.add(0, 1) -> [0, 1]
// rs.add(3, 4) -> [0, 1], [3, 4]
// rs.add(4, 5) -> [0, 1], [3, 5]
// rs.add(1, 3) -> [0, 5]
//
// Any addition has 5 possible situations:
// 1. Overlaps with nothing -> Append a new range
// 2. and 3. Overlaps either on the left or on the right -> extend the existing range
// 4. Overlaps with both -> Merge both ranges
// 5. Included within one -> Do nothing
//
// I was stuck on missing case 6:
// 6. Doesn't overlap with any edges, but includes one or more ranges within.
// -> Remove these ranges.

// Doubly linked list
// Note from a month later: there's no need for this to be a LL.

struct RSRange { ull lo, hi; };
struct RangeSet { struct RSRange arr[N_RANGES]; ui n; };

/* Searches for overlapping ranges.
 * 'below' is the index of a node in RS where [a, b] in below st. lo in [a, b],
 * if one exists. Same for 'above', but with "hi in [a, b]".
 *
 * If [lo, hi] in [a, b] in RS, then above = below. */
struct RSOverlap { ui below, above; };
struct RSOverlap find_overlap(struct RangeSet *rs, ull lo, ull hi) {
	struct RSOverlap overlap = { .below = -1, .above = -1 };
	for (ui i=0; i<rs->n; i++) {
		struct RSRange node = rs->arr[i];
		if (node.lo <= lo && lo <= node.hi)
			overlap.below = i;
		if (node.lo <= hi && hi <= node.hi)
			overlap.above = i;
	}
	return overlap;
}

void rs_append(struct RangeSet *rs, ull lo, ull hi) {
	// TODO overflow check?
	rs->arr[rs->n++] = (struct RSRange) { .lo = lo, .hi = hi };
}

void rs_remove(struct RangeSet *rs, ui i) {
	rs->n--;
	rs->arr[i] = rs->arr[rs->n];
}
/* Removes any ranges R in RS where R in [lo, hi]. */
void rs_remove_enclosed(struct RangeSet *rs, ull lo, ull hi) {
	for (ui i=0; i<rs->n; i++) {
		struct RSRange node = rs->arr[i];
		if (lo <= node.lo && node.hi <= hi)
			rs_remove(rs, i);
	}
}

void rs_add(struct RangeSet *rs, ull lo, ull hi) {
	struct RSOverlap overlap = find_overlap(rs, lo, hi);

	// If the ends of [lo, hi] touch no ends of another range
	if (overlap.above == -1 && overlap.below == -1) {
		// Remove any ranges enclosed by [lo, hi] and append.
		rs_remove_enclosed(rs, lo, hi);
		rs_append(rs, lo, hi);
	} else {
		if (overlap.above != overlap.below) {
			/* Now we have: [lo, hi] overlaps below or above,
			 * or on both with different ranges */
			if (overlap.above != -1 && overlap.below != -1) {
				ull xlo = rs->arr[overlap.below].lo;
				ull xhi = rs->arr[overlap.above].hi;
				rs_remove(rs, overlap.above);
				rs_remove(rs, overlap.below);
				rs_append(rs, xlo, xhi);
			} else if (overlap.above != -1 && overlap.below == -1) {
				rs->arr[overlap.above].lo = lo;
			} else if (overlap.above == -1 && overlap.below != -1) {
				rs->arr[overlap.below].hi = hi;
			}
		}
	}
}
void rs_display(struct RangeSet *rs) {
	for (ui i=0; i<rs->n; i++) {
		struct RSRange range = rs->arr[i];
		printf("[%llu, %llu]\n", range.lo, range.hi);
	}
}
ull rs_count_included(struct RangeSet *rs) {
	ull count = 0;
	for (ui i=0; i<rs->n; i++) {
		struct RSRange range = rs->arr[i];
		count += range.hi - range.lo + 1;
	}
	return count;
}

ull p2(struct Database *db) {
	struct RangeSet rs = {0};
	for (ui i=0; i<N_RANGES; i++)
		rs_add(&rs, db->fresh_los[i], db->fresh_his[i]);
	return rs_count_included(&rs);
}

int main(void) {
	struct Database db = {0};
	parse(&db);

	printf("p1: %llu\n", p1(&db));
	printf("p2: %llu\n", p2(&db));
}
