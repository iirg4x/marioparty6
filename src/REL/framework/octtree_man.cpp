// Loads archived octree geometry and provides leaf triangles for debug drawing.
static const char rcsid[] = "$Id: octtree_man.cpp,v 1.22 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/octtree_man.h"

extern "C" {
#include <stdio.h>
}

// The Octree constructor reads scalar counts from resources that must exist in the archive.
template <class Value>
inline Value octreeArchiveValue(OctreeArchive *archive, const char *resourceName)
{
    return *(Value *)archive->find(resourceName);
}

// When an octree is created, bind its geometry tables and initialize its archived nodes.
Octree::Octree(OctreeArchive *archiveResource) : archive(archiveResource)
{
    OctreeArchive *geometryArchive = getArchive();
    triangles = (OctreeTriangle *)geometryArchive->find("TriangleTable");
    triangleCount = octreeArchiveValue<long>(geometryArchive, "TriangleCount");
    vertices = (Vec *)geometryArchive->find("VertexTable");
    vertexCount = octreeArchiveValue<long>(geometryArchive, "VertexCount");
    nodes = (OctreeDebugNode *)geometryArchive->find("OctTable");
    triangleIndices = (unsigned short *)geometryArchive->find("TriIdxTable");
    // Replace child indices with pointers in the archive's node table itself.
    initializeOctreeNodes(nodes, octreeArchiveValue<long>(geometryArchive, "OctCount"));
    printf("vtx:%d tri:%d\n", vertexCount, triangleCount);
}

// Debug drawing constructs an iterator over the triangle-index range of one selected leaf.
OctreeTriangleIterator::OctreeTriangleIterator(Octree *tree, OctreeDebugNode *node)
{
    triangles = tree->triangleTable();
    vertices = tree->vertexTable();
    unsigned short *leafIndex = tree->indexTable();
    leafIndex += node->triangleOffset;
    current = leafIndex;
    leafIndex += node->triangleCount;
    end = leafIndex;
}

// drawNode calls this before each triangle; expose its vertices and debug color index.
// Exhaustion returns false without changing the last triangle's exposed values.
bool OctreeTriangleIterator::next()
{
    if (current >= end) return false;
    // Leaf entries index the shared triangle table, whose records index the vertex table.
    OctreeTriangle *triangle = &triangles[*current++];
    a = &vertices[triangle->vertex[0]];
    b = &vertices[triangle->vertex[1]];
    c = &vertices[triangle->vertex[2]];
    color = (unsigned short)triangle->color;
    return true;
}
