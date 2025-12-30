#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#define $PANIC(lit) do { puts("Panic: " lit); exit(1); } while (0)
#define $EXPECT(cond, lit) do {if (!(cond)) $PANIC(lit); } while (0)
typedef unsigned long long ull;

/* -- Input representation --
 * The input is an edge list of a directed graph. We will treat the number of
 * lines as the number of nodes, as we are not interested in possible nodes
 * that do not have further destinations, except 'out'. Input is assumed
 * acyclic. */

/*
char input[] = {
#embed "example.txt"
};
#define N_NODES 10
*/
char input[] = {
#embed "input.txt"
};
#define N_NODES 581

/* -- Input repr, ids --
 * The nodes have three-letter ids (lowercase A-Z).
 * These will be bijectionally mapped to [0, 26^3-1]. */
#define $LNUM(c) (c-'a')
#define $SQUARE(x) (x*x)
ull idnum(char id[3]) {
	char a = id[0], b = id[1], c = id[2];
	return $LNUM(a) * $SQUARE(26) + $LNUM(b) * 26 + $LNUM(c);
}

unsigned long hash(unsigned long x) {
	x = ((x >> 16) ^ x) * 0x119de1f3u;
    x = ((x >> 16) ^ x) * 0x119de1f3u;
    x = (x >> 16) ^ x;
    return x;
}

/* -- Insertion-only hash-reindex --
 * This data structure maps each seen ull to the lowest unsigned integer
 * possible. It implementes O(1) insertion and lookup, assuming the number of
 * items is bounded. Looking up a value inserts it. */

/* 'HashRemapItem' is a linked list node. */
struct HashReindexItem { ull key, value; struct HashReindexItem *next; };
struct HashReindex {
	struct HashReindexItem **heads, *pool; ull heads_c, pool_l, pool_c;
};
/* Map is a combination of insertion and lookup.
 * The data structure is mutated to insert the key, if it does not exist. */
ull hr_map(struct HashReindex *hr, ull key) {
	struct HashReindexItem **link = &hr->heads[hash(key) % hr->heads_c];
	while (*link) {
		struct HashReindexItem *node = *link;
		if (node->key == key)
			return node->value;
		link = &node->next;
	}
	// INSERT
	$EXPECT(hr->pool_l < hr->pool_c, "hash-remap pool ran out of space");
	struct HashReindexItem *node = &hr->pool[hr->pool_l];
	node->key = key;
	node->value = hr->pool_l;
	node->next = NULL;
	hr->pool_l++;
	*link = node;
	return node->value;
}
#define CONNECTION_CAP 30
struct ConnectionA { ull keys[CONNECTION_CAP], n; };

#define $ISAL(x) ('a' <= x && x <= 'z')
#define $VALID(c) ($ISAL(*c) && $ISAL(*(c+1)) && $ISAL(*(c+1)))
#define $CONSUME_ID do {\
	if (!($VALID(c))) $PANIC("parser expected a lowercase id");\
	id[0] = *c; id[1] = *(c+1); id[2] = *(c+2); c += 3;\
} while (0)
#define $CONSUME(x) do {while (*c == x) c++;} while (0)
void parse(struct HashReindex *hr, struct ConnectionA *conns) {
	char *c = input, id[3];
	for (ull i=0; i<N_NODES; i++) {
		$CONSUME_ID;
		ull node = idnum(id);
		struct ConnectionA *node_conns = &conns[hr_map(hr, node)];
		while (*c == ':')
			c++;
		while (*c != '\n') {
			$CONSUME(' ');
			$CONSUME_ID;
			ull nbor = idnum(id);
			node_conns->keys[node_conns->n++] = nbor;
			$CONSUME(' ');
		}
		$CONSUME('\n');
	}
}

/* -- P1 --
 * Let W(X): Node -> Int denote the ways to get to our destination D from X.
 * W(D) := 1, and W(X) := sum of W(Y) st. X->Y exists. Memoise.
 * Turns out P1 was small enough that the memoisation makes no difference. */

struct WMemo { ull values[N_NODES]; bool xists[N_NODES]; };
ull W(
	struct HashReindex *hr, struct WMemo *memo,
	const struct ConnectionA *conns, ull key
) {
	if (key == idnum("out"))
		return 1;
	ull n = hr_map(hr, key);
	if (memo->xists[n])
		return memo->values[n];
	ull sum = 0;
	for (ull i=0; i<conns[n].n; i++)
		sum += W(hr, memo, conns, conns[n].keys[i]);
	memo->xists[n] = true;
	memo->values[n] = sum;
	return sum;
}

/* P2 can be solved almost the same as P1, but the argument must include 
 * whether the path has encountered the two middle nodes. This must of course
 * be memoised as well.
 * The two booleans make four combinations of values. This is encoded with
 * an array of 4*N_NODES entries. See 'zmemo_n' for the index mapping. */
struct ZMemo { ull values[4 * N_NODES]; bool xists[4 * N_NODES]; };
ull zmemo_n(ull key, bool sDAC, bool sFFT) {
	return sDAC * N_NODES + sFFT * N_NODES + key;
}
ull Z(
	struct HashReindex *hr, struct ZMemo *memo, const struct ConnectionA *conns,
	ull key, bool sDAC, bool sFFT
) {
	sDAC |= key == idnum("dac");
	sFFT |= key == idnum("fft");
	if (key == idnum("out"))
		return sDAC && sFFT;
	ull n = hr_map(hr, key);
	ull m = zmemo_n(n, sDAC, sFFT);
	if (memo->xists[m])
		return memo->values[m];
	ull sum = 0;
	for (ull i=0; i<conns[n].n; i++)
		sum += Z(hr, memo, conns, conns[n].keys[i], sDAC, sFFT);
	memo->xists[m] = true;
	memo->values[m] = sum;
	return sum;
}

/* Capacity of the hash map LL-head array */
#define N_HEADS (N_NODES * 2)
int main(void) {
	struct HashReindexItem hr_pool[N_NODES] = {0}, *hr_heads[N_HEADS] = {0};
	struct HashReindex hr = {
		.heads = hr_heads, .heads_c = N_HEADS,
		.pool = hr_pool, .pool_c = N_NODES, .pool_l = 0
	};
	struct ConnectionA connections[N_NODES] = {0};
	parse(&hr, connections);

	struct WMemo wmemo = {0};
	printf("p1: %llu\n", W(&hr, &wmemo, connections, idnum("you")));
	/* Any paths from 'svr' -> 'out' */
	// printf("svr->out: %llu\n", W(&hr, &wmemo, connections, idnum("svr")));
	struct ZMemo zmemo = {0};
	printf("p2: %llu\n", Z(&hr, &zmemo, connections, idnum("svr"), false, false));
	return 0;
}
