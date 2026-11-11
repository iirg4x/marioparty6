// Selected-leaf sets and model callbacks used to draw octree triangle outlines.
#ifndef FRAMEWORK_OCTTREE_DEBUG_H
#define FRAMEWORK_OCTTREE_DEBUG_H
#include <algorithm>
#include <functional>
#include <set>

#include "REL/framework/octtree_man.h"
#include "REL/framework/model.h"

extern "C" {
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
}

// Ordered selection of leaves; the entries do not own the leaf geometry.
struct OctreeNodeSet : std::set<OctreeDebugNode *> {};
#endif
