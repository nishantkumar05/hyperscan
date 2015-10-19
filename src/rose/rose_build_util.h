/*
 * Copyright (c) 2015, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ROSE_BUILD_UTIL_H
#define ROSE_BUILD_UTIL_H

#include "rose_graph.h"
#include "util/graph.h"

#include <algorithm>

namespace ue2 {

// Calculate the minimum depth for the given set of vertices, ignoring those
// with depth 1.
template<class Cont>
static
u8 calcMinDepth(const std::map<RoseVertex, u32> &depths, const Cont &verts) {
    u8 d = 255;
    for (RoseVertex v : verts) {
        u8 vdepth = (u8)std::min((u32)255, depths.at(v));
        if (vdepth > 1) {
            d = std::min(d, vdepth);
        }
    }
    return d;
}

// Comparator for vertices using their index property.
struct VertexIndexComp {
    VertexIndexComp(const RoseGraph &gg) : g(gg) {}

    bool operator()(const RoseVertex &a, const RoseVertex &b) const {
        const RoseVertexProps &pa = g[a];
        const RoseVertexProps &pb = g[b];

        if (pa.idx < pb.idx) {
            return true;
        }
        if (pa.idx > pb.idx) {
            return false;
        }

        assert(a == b); // All vertex indices should be distinct.
        return a < b;
    }

    const RoseGraph &g;
};

// Vertex set type, ordered by index. Construct with a graph reference.
typedef std::set<RoseVertex, VertexIndexComp> RoseVertexSet;

/**
 * \brief Add two Rose depths together, coping correctly with infinity at
 * ROSE_BOUND_INF.
 */
static inline
u32 add_rose_depth(u32 a, u32 b) {
    assert(a <= ROSE_BOUND_INF);
    assert(b <= ROSE_BOUND_INF);

    if (a == ROSE_BOUND_INF || b == ROSE_BOUND_INF) {
        return ROSE_BOUND_INF;
    }

    u32 rv = a + b;
    assert(rv >= a && rv >= b);
    return rv;
}

} // namespace ue2

#endif // ROSE_BUILD_UTIL_H
