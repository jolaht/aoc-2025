#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
typedef unsigned long long ull;
typedef unsigned int ui;
#define $PANIC(lit) do { puts("Panic: " lit); exit(1); } while (0)
#define $EXPECT(cond, lit) do {if (!(cond)) $PANIC(lit); } while (0)

/*
char input[] = {
#embed "example.txt"
};
#define N_BOXES 20
#define N_CONNECTIONS 10
*/
char input[] = {
#embed "input.txt"
};
#define N_BOXES 1000
#define N_CONNECTIONS N_BOXES

#define N_BIGGEST 3
#define N_COMBINATIONS (N_BOXES * (N_BOXES - 1) / 2)

void panic(char *msg) { printf("Panic: %s\n", msg); exit(1); }

struct DSN { ui size, parent; };
struct DisjointSet { struct DSN arr[N_BOXES]; size_t nsets; };

void ds_init(struct DisjointSet *ds) {
	ds->nsets = N_BOXES;
	for (ui i=0; i<N_BOXES; i++)
		ds->arr[i] = (struct DSN) {.size = 1, .parent = UINT_MAX};
}
ui ds_find(struct DisjointSet *ds, ui i) {
	if (ds->arr[i].parent != UINT_MAX) {
		ui p = ds_find(ds, ds->arr[i].parent);
		ds->arr[i].parent = p;
		return p;
	}
	return i;
}
void ds_union(struct DisjointSet *ds, ui x, ui y) {
	ui rxi = ds_find(ds, x), ryi = ds_find(ds, y);
	if (rxi == ryi)
		return;
	struct DSN *rx = &ds->arr[rxi], *ry = &ds->arr[ryi];
	if (rx->size < ry->size) {
		rx->parent = ryi;
		ry->size += rx->size;
	} else {
		ry->parent = rxi;
		rx->size += ry->size;
	}
	ds->nsets--;
}


struct HeapNode { ull key, value; };
struct MinHeap { struct HeapNode *arr; ui l, cap; };
ui heap_child_left(ui i) { return 2*i+1; }
ui heap_child_right(ui i) { return 2*i+2; }
ui heap_parent(ui i) {
	$EXPECT(i>0, "heap_parent called for the root of the heap");
	return (i-1)/2;
}
void heap_swap(struct HeapNode *x, struct HeapNode *y) {
	struct HeapNode tmp = *x;
	*x = *y;
	*y = tmp;
}
void upheap(struct MinHeap *heap, ui i) {
	if (i == 0)
		return;
	$EXPECT(i < heap->l, "upheap called for a non-existent node");
	size_t pi = heap_parent(i);
	struct HeapNode *this = &heap->arr[i], *parent = &heap->arr[pi];
	if (this->key < parent->key) {
		heap_swap(this, parent);
		upheap(heap, pi);
	}
}
void downheap(struct MinHeap *heap, size_t i) {
	$EXPECT(i < heap->l, "downheap called for a non-existent node");
	size_t li = heap_child_left(i), ri = heap_child_right(i);
	if (li >= heap->l)
		return;
	struct HeapNode *this = &heap->arr[i], *left = &heap->arr[li];
	if (ri >= heap->l) {
		if (left->key < this->key)
			heap_swap(left, this);
		return;
	}
	struct HeapNode *right = &heap->arr[ri];
	if (this->key < right->key && this->key < left->key)
		return;
	size_t j = left->key < right->key ? li : ri;
	heap_swap(this, &heap->arr[j]);
	downheap(heap, j);
}
void heapify(struct MinHeap *heap) {
	for (size_t i = heap_parent(heap->l-1); i-->0; )
		downheap(heap, i);
}
struct HeapNode heap_dequeue(struct MinHeap *heap) {
	$EXPECT(heap->l > 0, "heap_dequeue called on an empty heap");
	struct HeapNode taken = heap->arr[0];
	heap->l--;
	heap->arr[0] = heap->arr[heap->l];
	downheap(heap, 0);
	return taken;
}

void parse(int boxes[][3]) {
	char *c = input; int delta;
	for (ui i=0; i<N_BOXES; i++) {
		int *box = boxes[i];
		sscanf(c, "%i,%i,%i%n", &box[0], &box[1], &box[2], &delta);
		c += delta;
	}
}

/* Euclidian distance squared */
ull edsq(int boxes[][3], ui i, ui j) {
	int *b1 = boxes[i], *b2 = boxes[j];
	long long int dx = b1[0]-b2[0], dy = b1[1]-b2[1], dz = b1[2]-b2[2];
	return dx*dx + dy*dy + dz*dz;
}
struct Distance { ui i, j; ull d; };
void build_distance_table(int boxes[][3], struct Distance distances[]) {
	ui l = 0;
	for (ui i=0; i<N_BOXES; i++) {
		for (ui j=0; j<N_BOXES; j++) {
			if (i<j) {
				struct Distance d = {i, j, edsq(boxes, i, j)};
				distances[l++] = d;
			}
		}
	}
}
void init_dist_heap_arr(struct Distance distances[], struct HeapNode qa[]) {
	for (ui i=0; i<N_COMBINATIONS; i++)
		qa[i] = (struct HeapNode) { .key = distances[i].d, .value = i };
}

void connect(struct DisjointSet *connections, struct Distance distance) {
	ds_union(connections, distance.i, distance.j);
}

ull product_of_biggest_circuit_sizes(struct DisjointSet *connections) {
	struct HeapNode root_heap_arr[N_BOXES];
	struct MinHeap root_heap = {
		.arr = root_heap_arr, .cap = N_BOXES, .l = 0
	};
	for (ui i=0; i<N_BOXES; i++) {
		if (i == ds_find(connections, i)) {
			root_heap_arr[root_heap.l++] = (struct HeapNode) {
				.key = UINT_MAX - connections->arr[i].size,
				.value = i
			};
		}
	}
	heapify(&root_heap);
	ull product = 1;
	for (ui i=0; i<N_BIGGEST; i++)
		product *= connections->arr[heap_dequeue(&root_heap).value].size;
	return product;
}

/* These arrays are both too large for the stack with the main input. */
struct Distance distances[N_COMBINATIONS];
struct HeapNode heap_arr[N_COMBINATIONS];

int main(void) {
	int boxes[N_BOXES][3];
	parse(boxes);
	struct DisjointSet connections = {0};
	ds_init(&connections);

	build_distance_table(boxes, distances);
	init_dist_heap_arr(distances, heap_arr);
	struct MinHeap heap = {
		.arr = heap_arr, .l = N_COMBINATIONS, .cap = N_COMBINATIONS
	};
	heapify(&heap);

	// P1
	for (ui i=0; i<N_CONNECTIONS; i++)
		connect(&connections, distances[heap_dequeue(&heap).value]);
	printf("p1: %llu\n", product_of_biggest_circuit_sizes(&connections));

	// P2
	struct Distance last;
	while (connections.nsets > 1) {
		last = distances[heap_dequeue(&heap).value];
		connect(&connections, last);
	}
	printf("p2: %llu\n", (ull) boxes[last.i][0] * boxes[last.j][0]);

	return 0;
}
