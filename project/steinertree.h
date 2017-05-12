#ifndef __STEINER_TREE_H
#define __STEINER_TREE_H

/* Edge between orig and end with weight value */
typedef struct {
        int orig;
        int end;
        int value;
} edge;

/* Heap to hold edges to return Min/Max weighted edge in O(logE) complexity */
typedef struct {
        int size;
        edge *e;
} heap;

/* Function: debugHeap
 * Arguments: Pointer to the heap Structure
 * 	Used to debug data in the heap
 */
void debugHeap(heap *);

/* Function: minHeapify
 * Arguments: Pointer to the heap Structure
 * 	Useed to heapify the data such that minimum element is at the root of the heap structure.
 */
void minHeapify(heap *);

/* Function: addToHeap
 * Arguments: Pointer to the heap Structure,
 *		int originatingVertex,
 *		int endVertex,
 *		int weightOfEdge
 */
void addToHeap(heap *, int , int , int );

/* Function: getMinFromHeap
 * Arguments: Pointer to the heap Structure
 * 	Used to get minimum element from the heap
 * Return Value: Pointer to the Edge structure
 */
edge *getMinFromHeap(heap *);

/* List of nodes in the path between two vertices */
typedef struct node {
        int v;
        struct node *next;
} path;

/* Function: findPath
 * Arguments: int numberOfVertices,
 * 		int edgeWeights[totalVertices][totalVertices],
 * 		int weightValue,
 * 		int startVertex,
 * 		int endVertex
 * Return Value: Pointer to the path of startVertex to endVertex with weightValue
 */
path *findPath(int vertices, int localWeights[][vertices], int weight, int startv, int endv);

/* Function: findLGraph
 * Arguments: int numberOfVertices,
 * 		int numberOfEdges,
 * 		int edgeweights[numberOfVertices][numberOfVertices],
 * 		int numberOfTerminals,
 * 		int *listOfTerminals,
 * 		<OUT> int pathLength[numberOfTerminals][numberOfTerminals]
 *	Function used to find the metric closure of an input graph with edgeweights and outputs
 *	shortest path lengths between vertices in listofterminals.
 */
void findLGraph(int vertices, int edges, int weights[][vertices],
                int *terminalVertices, int terminals, int pathLength[][terminals]);

/* Function: getMinSpanTree
 * Arguments: int numberOfTerminals,
 * 		int listOfTerminals[numberOfTerminals],
 * 		int edgeweights[numberOfTerminals][numberOfTerminals],
 * 		<OUT>int tree[numberOfTerminals][numberOfTerminals]
 *	Function to find the minimum spanning tree of a given graph with weights and outputs the tree.
 * 	Algorithm followed: PRIM'S MINIMUM SPANNING TREE
 */
void getMinSpanTree(int terminals, int *terminalVertices, int weights[][terminals], int tree[][terminals]);

/* Function: findSteinerTree
 * Arguments: int numberOfTerminals,
 * 		int listOfTerminals[numberOfTerminals],
 * 		int minSpanTree[numberOfTerminals][numberOfTerminals],
 * 		int numberOfVertices,
 * 		int edgeweights[numberOfVertices][numberOfVertices],
 * 		<OUT> int minSteinerTree[numberOfVertices][numberOfVertices]
 *	Function used to get the minimum steiner tree from given minimum spanning tree of metric closure obtained
 *	from graph with edgeweights.
 *	Outputs the edgeweights of terminals and steiner points used in the minimum steiner tree.
 */
void findSteinerTree(int terminals, int *terminalVertices, int minSpanTree[][terminals],
                        int vertices, int localWeights[][vertices], int minSteinerTree[][vertices]);
#endif
