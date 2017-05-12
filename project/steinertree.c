#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "steinertree.h"

/* Function: debugHeap
 * Arguments: Pointer to the heap Structure
 *      Used to debug data in the heap
 */
void debugHeap(heap *minHeap) {
	int i = 1;
	printf("Heap:");
	while (i <= minHeap->size) {
		printf("%d %d->%d, ", minHeap->e[i].value, minHeap->e[i].orig, minHeap->e[i].end);
		i+=1;
	}
	printf("\n");
}

/* Function: minHeapify
 * Arguments: Pointer to the heap Structure
 *      Useed to heapify the data such that minimum element is at the root of the heap structure.
 */
void minHeapify(heap *minheap) {
	int size = minheap->size;
	edge value = minheap->e[size];
	int parent = size/2;
	int child = size;
	int done = 0;
	while (parent != 0 && !done) {
		/* edge structre contains value, originating vertex, and end vertex.
		 * if parent value is greater than child value, min heap property is violated
		 * so fix by swapping child and parent. 
		 * Continue until no such violation is found or we reach root */
		if (minheap->e[parent].value > value.value) {
			edge temp = minheap->e[parent];
			minheap->e[parent].value = value.value;
			minheap->e[parent].orig = value.orig;
			minheap->e[parent].end = value.end;
			minheap->e[child].value = temp.value;
			minheap->e[child].orig = temp.orig;
			minheap->e[child].end = temp.end;
			child = parent;
			parent = parent/2;
		} else {
			// no element violates min heap property
			done = 1;
		}
	}
}

/* Function: addToHeap
 * Arguments: Pointer to the heap Structure,
 *              int originatingVertex,
 *              int endVertex,
 *              int weightOfEdge
 */
void addToHeap(heap *minheap, int value, int orig, int end) {
	int size = minheap->size;
	// Add the element to the end of the list and heapify to make sure
	// elements satisfy min heap property
	minheap->e[size+1].value = value;
	minheap->e[size+1].orig = orig;
	minheap->e[size+1].end = end;
	minheap->size = size + 1;
	minHeapify(minheap);
}

/* Function: getMinFromHeap
 * Arguments: Pointer to the heap Structure
 *      Used to get minimum element from the heap
 * Return Value: Pointer to the Edge structure
 */
edge *getMinFromHeap(heap *minheap) {
	int size = minheap->size;
	if (size == 0) {
		return NULL;
	}
	// Element at the first index is the minimum element in the min heap
	edge *value = (edge *)malloc(sizeof(edge));
	value->value = minheap->e[1].value;
	value->orig = minheap->e[1].orig;
	value->end = minheap->e[1].end;
	// copy the last element in the heap
	minheap->e[1].value = minheap->e[size].value;
	minheap->e[1].orig = minheap->e[size].orig;
	minheap->e[1].end = minheap->e[size].end;
	minheap->size = size - 1;
	size--;

	int i = 1;
	while (i < size) {
		// Make sure min heap property is satisfied on removing the root of the heap.
		if (2*i > size || 2*i+1 > size) {
			break;
		}
		int v = minheap->e[i].value;
		int orig = minheap->e[i].orig;
		int end = minheap->e[i].end;
		int lchild = minheap->e[2*i].value;
		int rchild = minheap->e[2*i +1].value;

		// check root with left child and right child.
		// if root value is greater than left child, left child is less than right child,
		// swap root and left child. Goto left and redo until end of heap or no violator is found.
		if (v > lchild && lchild <= rchild) {
			minheap->e[i].value = minheap->e[2*i].value;
			minheap->e[i].orig = minheap->e[2*i].orig;
			minheap->e[i].end = minheap->e[2*i].end;
			minheap->e[2*i].value = v;
			minheap->e[2*i].orig = orig;
			minheap->e[2*i].end = end;
			i = 2*i;
		} else if (v > rchild && rchild <= lchild) {
			// if root value is greater than right child, right child is less than left child,
			// swap root and right child. Goto right and redo until end of heap or no violator is found.
			minheap->e[i].value = minheap->e[2*i+1].value;
			minheap->e[i].orig = minheap->e[2*i+1].orig;
			minheap->e[i].end = minheap->e[2*i+1].end;
			minheap->e[2*i+1].value = v;
			minheap->e[2*i+1].orig = orig;
			minheap->e[2*i+1].end = end;
			i = 2*i + 1;
		} else {
			// no violator of min heap property is found
			break;
		}
	}
	return value;
}

/* Function: findLGraph
 * Arguments: int numberOfVertices,
 *              int numberOfEdges,
 *              int edgeweights[numberOfVertices][numberOfVertices],
 *              int numberOfTerminals,
 *              int *listOfTerminals,
 *              <OUT> int pathLength[numberOfTerminals][numberOfTerminals]
 *      Function used to find the metric closure of an input graph with edgeweights and outputs
 *      shortest path lengths between vertices in listofterminals.
 */
void findLGraph(int vertices, int edges, int weights[][vertices],
		int *terminalVertices, int terminals, int pathLength[][terminals]) {
	int i = 0;
	int j = 0;
	// initialize pathLengths(edge weights of terminals) with edge weights.
	for (i = 0; i < terminals; i++) {
		for (j = 0; j < terminals; j++) {
			if (i == j) pathLength[i][j] = 0;
			pathLength[i][j] = weights[terminalVertices[i]][terminalVertices[j]];
		}
	}
	int k = 0;
	int l = 0;
	// Run bellman Ford-Fulkerson algorithm to get the minimum path between two vertices in the original graph.
	// Repeat until total number of edges since path cannot contain more than total number of edges.
	while (k < edges) {
		for (i = 0; i < vertices; i++) {
			for (j = 0; j < vertices; j++) {
				int min = weights[i][j];
				for (l = 0; l < vertices; l++) {
					// if weights are not invalid, check if the addition of vertex l reduces the path length
					// between i and j. If so, update the new minimum.
					if (weights[i][l] != INT_MAX && weights[l][j] != INT_MAX) {
						int path = weights[i][l] + weights[l][j];
						if (path < min) {
							min = path;
						}
					}
				}
				weights[i][j] = min;
			}
		}
		k++;
	}
}

/* Function: getMinSpanTree
 * Arguments: int numberOfTerminals,
 *              int listOfTerminals[numberOfTerminals],
 *              int edgeweights[numberOfTerminals][numberOfTerminals],
 *              <OUT>int tree[numberOfTerminals][numberOfTerminals]
 *      Function to find the minimum spanning tree of a given graph with weights and outputs the tree.
 *      Algorithm followed: PRIM'S MINIMUM SPANNING TREE
 */
void getMinSpanTree(int terminals, int *terminalVertices, int weights[][terminals], int tree[][terminals]) {
	if (terminals == 1) {
		tree[0][0] = weights[terminalVertices[0]][terminalVertices[0]];
		return;
	}
	// min heap used to get the minimum weighted edge in the list of edges added to the heap.
	heap minHeap;
	minHeap.size = 0;
	minHeap.e = (edge *)malloc(sizeof(edge) * (terminals*terminals+1));

	// boolean used to avoid cycles while adding edge ending at the vertex i.
	// if 1, donot add the edge. If 0, add the edge and set the boolean to 1.
	int visited[terminals];
	int i = 0;
	int j = 0;
	int edges = 0;
	for (i = 0; i < terminals; i++) {
		for (j = 0; j < terminals; j++) {
			if (weights[i][j] != 0 && weights[i][j] != INT_MAX) {
				edges++;
			}
		}
		visited[i] = 0;
	}

	int edgesAdded = 0;
	int startVertex = 0;
	// add edges until max edges reached
	while (edgesAdded < edges) {
		// PRIM's Algorithm
		// get the minimum weighted edge, check if it creates a cycle in the tree.
		// If so, donot add the edge and get next minimum edge.
		edge *e = getMinFromHeap(&minHeap);
		while (e != NULL && visited[e->end] == 1) {
			e = getMinFromHeap(&minHeap);
		}
		// If not, add the edge to the tree.
		if (e !=  NULL) {
			tree[e->orig][e->end] = e->value;
			startVertex = e->end;
			free(e);
			e = NULL;
		}
		// add the edges spawning from the newly added startVertex to the heap.
		for (j = 0; j < terminals; j++) {
			if (startVertex != j) {
				addToHeap(&minHeap, weights[startVertex][j], startVertex, j);
			}
		}
		visited[startVertex] = 1;
		edgesAdded += 1;
	}
	free(minHeap.e);
	minHeap.e = NULL;
}

/* Function: findPath
 * Arguments: int numberOfVertices,
 *              int edgeWeights[totalVertices][totalVertices],
 *              int weightValue,
 *              int startVertex,
 *              int endVertex
 * Return Value: Pointer to the path of startVertex to endVertex with weightValue
 */
path *findPath(int vertices, int localWeights[][vertices], int weight, int startv, int endv) {
	if (localWeights[startv][endv] == weight) {
		// if the path contains a single edge, create the path of startv->endv
		// and return the path.
		path *p = (path *)malloc(sizeof(path));
		p->v = startv;
		p->next = (path *)malloc(sizeof(path));
		p->next->v = endv;
		p->next->next = NULL;
		return p;
	} else {
		int i = startv;
		int j = 0;

		// Do a Depth First Search to get the path with value of weight input and return
		// the path.
		for (j = 0; j < vertices; j++) {
			if (j != i && j != endv) {
				int w = weight - localWeights[i][j];
				if (w < 0) {
					continue;	
				}
				// Recursive function DFS to get the path.
				// if i->j is the first part of the path, find j->endv path
				path *p = findPath(vertices, localWeights, w, j, endv);
				if (p != NULL) {
					// Found the path. Current path i->j.
					// Add j->endv to the current path and return the path
					path *lp = (path *)malloc(sizeof(path));
					lp->v = startv;
					lp->next = p;
					path *tmp = lp;
					while (tmp != NULL) {
						tmp = tmp->next;
					}
					return lp;
				}
			}
		}
		return NULL;
	}
}

/* Function: findSteinerTree
 * Arguments: int numberOfTerminals,
 *              int listOfTerminals[numberOfTerminals],
 *              int minSpanTree[numberOfTerminals][numberOfTerminals],
 *              int numberOfVertices,
 *              int edgeweights[numberOfVertices][numberOfVertices],
 *              <OUT> int minSteinerTree[numberOfVertices][numberOfVertices]
 *      Function used to get the minimum steiner tree from given minimum spanning tree of metric closure obtained
 *      from graph with edgeweights.
 *      Outputs the edgeweights of terminals and steiner points used in the minimum steiner tree.
 */
void findSteinerTree(int terminals, int *terminalVertices, int minSpanTree[][terminals],
			int vertices, int localWeights[][vertices], int minSteinerTree[][vertices]) {
	int i = 0;
	int j = 0;

	for (i = 0; i < terminals; i++) {
		for (j = 0; j < terminals; j++) {
			// If the minimum spanning tree of the metric closure of terminals contains valid edge weight,
			// expand the edge to the get the original path which might contain any steiner point.
			// On expansion, add the edges in the path to the steiner tree.
			if (minSpanTree[i][j] > 0 && minSpanTree[i][j] < INT_MAX) {
				path *p = findPath(vertices, localWeights, minSpanTree[i][j], terminalVertices[i], terminalVertices[j]);
				if (p != NULL) {
					path *tmp = p->next;
					path *prvtmp = p;
					while (tmp != NULL) {
						// undirected steiner tree
						minSteinerTree[prvtmp->v][tmp->v] = localWeights[prvtmp->v][tmp->v];
						minSteinerTree[tmp->v][prvtmp->v] = localWeights[tmp->v][prvtmp->v];
						prvtmp = tmp;
						tmp = tmp->next;
					}
					free(p);
					p = NULL;
					tmp = NULL;
					prvtmp = NULL;
				}
			}
		}
	}
}

int main() {
	int nVertices = 0; // total number of vertices in the graph
	int nEdges = 0; // total number of edges in the graph
	int nTerminals = 0; // total number of terminals in the graph

	// get the number of vertices, edges and terminals
	scanf("%d %d %d", &nVertices, &nEdges, &nTerminals);

	int weights[nVertices][nVertices]; // edge weights between i and j of vertices
	int minSteinerTree[nVertices][nVertices]; // edge weights of minimum steiner tree
	int localWeights[nVertices][nVertices]; // replica of edge weights of the original graph
	int terminalVertices[nTerminals]; // list of terminal vertices
	int pathLength[nTerminals][nTerminals]; // edge weights in the metric closure of terminal vertices over original graph
	int minSpanTree[nTerminals][nTerminals]; // edge weights in the minimum spanning tree of the metric closure of terminal vertices

	// array indices
	int i = 0;
	int j = 0;

	// initialization of edge weights and its replica
	for (i = 0; i < nVertices; i++) {
		for (j = 0; j < nVertices; j++) {
			if (i == j) {
				weights[i][j] = 0;
				minSteinerTree[i][j] = 0;
			}
			else {
				weights[i][j] = INT_MAX;
				minSteinerTree[i][j] = INT_MAX;
			}
		}
	}

	// get the edge weights of each edge
	for (i = 0; i < nEdges; i++) {
		int orig;
		int end;
		int weight;
		scanf("%d %d %d", &orig, &end, &weight);
		weights[orig][end] = weight;
		// using undirected graph
		weights[end][orig] = weight;
	}

	// get the terminal vertices
	for (i = 0; i < nTerminals; i++) {
		scanf("%d", &terminalVertices[i]);
	}

	// backup copy of weights
	for (i = 0; i < nVertices; i++) {
		for (j = 0; j < nVertices; j++) {
			localWeights[i][j] = weights[i][j];
		}
	}

	// get the metric closure of the original graph
	// shortest path between every pair of nodes is defined as an edge in the metric closure
	findLGraph(nVertices, nEdges, weights, terminalVertices, nTerminals, pathLength);
	printf("LGraph: \n");
	for (i = 0; i < nTerminals; i++) {
		for (j = 0; j < nTerminals; j++) {
			pathLength[i][j] = weights[terminalVertices[i]][terminalVertices[j]];
			printf("%d--->%d: %d 	", terminalVertices[i], terminalVertices[j], pathLength[i][j]);
			minSpanTree[i][j] = INT_MAX;
		}
		printf("\n");
	}

	// get the minimum spanning tree of the metric closure of terminals.
	getMinSpanTree(nTerminals, terminalVertices, pathLength, minSpanTree);
	printf("Min Span of LGraph:\n");
	for (i = 0; i < nTerminals; i++) {
		for (j = 0; j < nTerminals; j++) {
			if (minSpanTree[i][j] != INT_MAX)
				printf("Edge: %d ---> %d: Weight: %d\n", terminalVertices[i], terminalVertices[j], minSpanTree[i][j]);
		}
	}

	// trivial case: number of terminals = 1
	if (nTerminals == 1) {
		printf("Edge: NONE Vertex: %d Weight: %d\n", terminalVertices[0], minSpanTree[0][0]);
		return 0;
	}

	// get the steiner tree using the minimum spanning tree of the metric closure over terminals
	findSteinerTree(nTerminals, terminalVertices, minSpanTree, nVertices, localWeights, minSteinerTree);
	printf("Found Steiner Tree:\n");
	for (i = 0; i < nVertices; i++) {
		for (j = 0; j < nVertices; j++) {
			if (minSteinerTree[i][j] != 0 && minSteinerTree[i][j] != INT_MAX) {
				printf("Edge: %d --> %d Weight: %d\n", i, j, minSteinerTree[i][j]);
			}
		}
	}
	printf("\n");

	return 0;
}
