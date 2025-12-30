#include <stdio.h>
#include <stdlib.h>
#define $MIN(a, b) (a < b ? a : b)
#define $MAX(a, b) (a > b ? a : b)
#define $PANIC(msg) do { puts("Panic: " msg); exit(1); } while (0);
typedef unsigned long long ull;

/*
#define N_TILES 8
char input[] = {
#embed "example.txt"
};
*/
#define N_TILES 496
char input[] = {
#embed "input.txt"
};

void parse(ull tiles[][2]) {
	char *c = input;
	for (ull i=0; i<N_TILES; i++) {
		int d;
		ull *t = tiles[i];
		sscanf(c, "%llu,%llu%n", &t[0], &t[1], &d);
		c += d;
	}
}

/* P1 is trivial. This description is regarding P2.
 *
 * We are given a polygon by points that trace its outline.
 * Empirically: The outline seems ortrhogonal, and non-overlapping.
 *
 * We want to do the same test as for P1, but filtering out those rectangles,
 * that do not fit inside the polygon. The polygon can be described by a list of
 * orthogonal line segments. A rectangle (formed by the end points of the simply
 * outlined polygon) is contained within the polygon iff. its inner rectangle
 * does not intersect with any polygon outline segments. The inner rectangle is
 * what is left, when the outline of the rectangle is subtracted.
 *
 * This reduces the problem to intersection detection between a rectangle and
 * an orthogonal line.
 *
 * Furthermore, a line segment and a square intersect iff. (1) the line segment
 * intersects any sides of the rectangle, OR (2), either of the end points of
 * the line segment are enclosed by the rectangle.
 * This further reduces the problem to intersection detection of orthogonal
 * line segment, and detection of a point being contained in a rectangle.
 *
 * Applying linear search on above, solution is O(n^3), where n is the amount
 * of tiles, which is reasonable with n<500. */

bool range_includes(ull lo, ull hi, ull x) { return lo<=x && x<=hi; }
bool ranges_overlap(ull lo1, ull hi1, ull lo2, ull hi2) {
	return range_includes(lo1, hi1, lo2) || range_includes(lo1, hi1, hi2);
}
struct Point { ull x, y; };
/* Orthogonal line segment. Axis is 'x' or 'y'.
 * Fixed is the height or width that stays constant.
 * Original points A, B are kept in memory, as they would have to be
 * computed later. */
struct LineSegment { char axis; ull fixed, lo, hi; struct Point A, B; };
struct LineSegment ols_from_points(struct Point A, struct Point B) {
	if (A.x == B.x) {
		return (struct LineSegment) {
			.axis = 'x', .fixed = A.x, .A = A, .B = B,
			.lo = $MIN(A.y, B.y), .hi = $MAX(A.y, B.y)
		};
	}
	if (A.y == B.y) {
		return (struct LineSegment) {
			.axis = 'y', .fixed = A.y, .A = A, .B = B,
			.lo = $MIN(A.x, B.x), .hi = $MAX(A.x, B.x)
		};
	}
	$PANIC("expected an orthogonal line");
}
/* $INCLUDES(a, b, c) iff. c in [a, b]. */
#define $INCLUDES(a, b, x) (a <= x && x <= b)
/* $OVERLAPS(a, b, c, d) iff. xists x st. (x in [a, b]) AND (x in [c, d]). */
#define $OVERLAPS(a, b, c, d) ($INCLUDES(a, b, c) || $INCLUDES(a, b, d))
static inline bool ls_intersect(struct LineSegment a, struct LineSegment b) {
	if (a.axis == b.axis) {
		return a.fixed == b.fixed && $OVERLAPS(a.lo, a.hi, b.lo, b.hi);
	} else {
		struct LineSegment *h = (a.axis == 'x' ? &a : &b);
		struct LineSegment *v = (a.axis == 'y' ? &a : &b);
		return $INCLUDES(h->lo, h->hi, v->fixed) &&
			$INCLUDES(v->lo, v->hi, h->fixed);
	}
}

/* Extremum-represented rectangle. */
struct ER { ull xlo, xhi, ylo, yhi; };
/* Constructs an extremum-rectangle from arbitrarily ordered points. */
static inline struct ER er_from_points(struct Point A, struct Point B) {
	return (struct ER) {
		.xlo = $MIN(A.x, B.x), .xhi = $MAX(A.x, B.x),
		.ylo = $MIN(A.y, B.y), .yhi = $MAX(A.y, B.y)
	};
}
static inline bool er_contains_point(struct ER rect, struct Point P) {
	return $INCLUDES(rect.xlo, rect.xhi, P.x) &&
		$INCLUDES(rect.ylo, rect.yhi, P.y);
}
/* Subtracts the walls of a rectangle from a rectangle. */
struct ER er_get_enclosed(struct ER rec) {
	return (struct ER) {
		.xlo = rec.xlo + 1, .xhi = rec.xhi - 1,
		.ylo = rec.ylo + 1, .yhi = rec.yhi - 1
	};
}

/* Orthogonal line-segment representation (of a rectangle). */
struct OLSR { struct LineSegment sides[4]; };
static inline struct OLSR olsr_from_er(struct ER r) {
	return (struct OLSR) {
		.sides = {
			{.axis = 'y', .fixed = r.ylo, .lo = r.xlo, .hi = r.xhi},
			{.axis = 'y', .fixed = r.yhi, .lo = r.xlo, .hi = r.xhi},
			{.axis = 'x', .fixed = r.xlo, .lo = r.ylo, .hi = r.yhi},
			{.axis = 'x', .fixed = r.xhi, .lo = r.ylo, .hi = r.yhi}
		}
	};
}

bool er_intersects_any_seg(struct ER rectangle, ull tiles[][2]) {
	/* Intersection between a rectangle and a line segment occurs iff.
	 * any side of the rectangle intersects seg, or if either point of seg is
	 * enclosed within the rectangle.*/
	struct OLSR olsr = olsr_from_er(rectangle);
	for (ull i=0; i<N_TILES; i++) {
		ull *t1 = tiles[i], *t2 = tiles[(i+1) % N_TILES];
		struct Point A = { .x = t1[0], .y = t1[1] };
		struct Point B = { .x = t2[0], .y = t2[1] };
		struct LineSegment seg = ols_from_points(A, B);

		bool b = ls_intersect(olsr.sides[0], seg) ||
			ls_intersect(olsr.sides[1], seg) ||
			ls_intersect(olsr.sides[2], seg) ||
			ls_intersect(olsr.sides[3], seg) ||
			er_contains_point(rectangle, seg.A) ||
			er_contains_point(rectangle, seg.B);
		if (b)
			return true;
	}
	return false;
}
ull er_area(struct ER r) { return (r.xhi-r.xlo+1) * (r.yhi-r.ylo+1); }

struct Solution { ull p1, p2; };
struct Solution solve(ull tiles[][2]) {
	struct Solution r = {0};
	for (ull i=0; i<N_TILES; i++) {
		struct Point A = { .x = tiles[i][0], .y = tiles[i][1] };
		for (ull j=0; j<N_TILES; j++) {
			if (i<j) {
				struct Point B = { .x = tiles[j][0], .y = tiles[j][1] };
				struct ER rect = er_from_points(A, B);
				ull area = er_area(rect);
				r.p1 = $MAX(area, r.p1);
				if (!er_intersects_any_seg(er_get_enclosed(rect), tiles))
					r.p2 = $MAX(area, r.p2);
			}
		}
	}
	return r;
}

int main(void) {
	ull tiles[N_TILES][2];
	parse(tiles);
	struct Solution sol = solve(tiles);
	printf("p1: %llu\np2: %llu\n", sol.p1, sol.p2);
	return 0;
}
