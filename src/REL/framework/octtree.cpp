// Resolves archived octree nodes and finds the leaf containing a point.
static const char rcsid[] = "$Id: octtree.cpp,v 1.22 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/octtree_man.h"
extern "C" {
#include "REL/framework/msl_assertion.h"
}

// Octree construction converts archived child indices into node pointers.
void initializeOctreeNodes(OctreeDebugNode *nodeTable, int nodeCount) {
    OctreeDebugNode *currentNode = nodeTable;
    OctreeDebugNode *endNode = nodeTable + nodeCount;
    while (currentNode != endNode) {
        for (int childIndex = 0; childIndex < 8; childIndex++) {
            // The archive stores an all-ones child index for an empty slot.
            if ((unsigned long)currentNode->children[childIndex] + 65536 == 65535) {
                currentNode->children[childIndex] = 0;
            } else {
                currentNode->children[childIndex] =
                    nodeTable + (unsigned long) currentNode->children[childIndex];
            }
        }
        currentNode++;
    }
}

// This helper tests a point against six ordered bounds; bounds store minimum then maximum X/Y/Z.
// Points on a box edge are inside.
static int octTreeBoundsContainPoint(const float *orderedBounds, float pointX, float pointY,
                                     float pointZ) {
    if (orderedBounds[0] > pointX) {
        return 0;
    }
    if (orderedBounds[1] > pointY) {
        return 0;
    }
    if (orderedBounds[2] > pointZ) {
        return 0;
    }
    if (orderedBounds[3] < pointX) {
        return 0;
    }
    if (orderedBounds[4] < pointY) {
        return 0;
    }
    if (orderedBounds[5] < pointZ) {
        return 0;
    }
    return 1;
}

// Octree::nodeAt calls this while selecting a point; descend to its containing leaf.
OctreeDebugNode *findOctreeLeaf(OctreeDebugNode *rootNode, float pointX, float pointY,
                                float pointZ) {
    unsigned char rootContainsPoint;
    unsigned char childContainsPoint;
    if (rootNode->bounds[0] > pointX) {
        rootContainsPoint = 0;
    } else if (rootNode->bounds[1] > pointY) {
        rootContainsPoint = 0;
    } else if (rootNode->bounds[2] > pointZ) {
        rootContainsPoint = 0;
    } else if (rootNode->bounds[3] < pointX) {
        rootContainsPoint = 0;
    } else if (rootNode->bounds[4] < pointY) {
        rootContainsPoint = 0;
    } else if (rootNode->bounds[5] < pointZ) {
        rootContainsPoint = 0;
    } else {
        rootContainsPoint = 1;
    }
    if (rootContainsPoint == 0) {
        return 0;
    }
    while (rootNode->children[0] != 0) {
        OctreeDebugNode *parentNode = rootNode;
        for (int childIndex = 0; childIndex < 8; childIndex++) {
            OctreeDebugNode *candidateNode = rootNode->children[childIndex];
            if (candidateNode->bounds[0] > pointX) {
                childContainsPoint = 0;
            } else if (candidateNode->bounds[1] > pointY) {
                childContainsPoint = 0;
            } else if (candidateNode->bounds[2] > pointZ) {
                childContainsPoint = 0;
            } else if (candidateNode->bounds[3] < pointX) {
                childContainsPoint = 0;
            } else if (candidateNode->bounds[4] < pointY) {
                childContainsPoint = 0;
            } else if (candidateNode->bounds[5] < pointZ) {
                childContainsPoint = 0;
            } else {
                childContainsPoint = 1;
            }
            if (childContainsPoint != 0) {
                rootNode = candidateNode;
                break;
            }
        }
        // An internal node must have a child for every point inside its bounds.
        (rootNode != parentNode) ? (void)0
            : __msl_assertion_failed("p != parent", "octtree.cpp", 75);
    }
    return rootNode;
}
