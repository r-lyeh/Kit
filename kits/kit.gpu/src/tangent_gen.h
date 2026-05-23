#pragma once
/*
 * tangent_gen.h  –  Per-vertex tangent generation for indexed triangle meshes
 *
 * Implements the standard UV-gradient / Lengyel method:
 *   For each triangle, compute dPos/dUV to get T and B contributions,
 *   accumulate into per-vertex sums, then orthonormalise and compute the
 *   bitangent sign (w) by comparing the handedness against the cross product.
 *
 * This matches MikkTSpace for well-formed meshes (no UV seams with conflicting
 * tangent directions).  For the DamagedHelmet and similar clean assets it
 * produces identical normals to what Blender/three.js would bake.
 *
 * Usage:
 *   tangent_gen(vertices, vertex_count, indices, index_count);
 *   // fills vertex[i].tangent[0..3] in-place
 */

#include "mesh.h"   /* Vertex */
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void tangent_gen(Vertex   *verts,  uint32_t  vc,
                        uint32_t *indices, uint32_t  ic)
{
    /* Accumulation buffers (float3 tan1 + float3 tan2 per vertex) */
    float *tan1 = (float*)calloc(vc * 3, sizeof(float));
    float *tan2 = (float*)calloc(vc * 3, sizeof(float));
    if (!tan1 || !tan2) { free(tan1); free(tan2); return; }

    uint32_t tri_count = ic / 3;
    for (uint32_t t = 0; t < tri_count; t++) {
        uint32_t i0 = indices[t*3+0];
        uint32_t i1 = indices[t*3+1];
        uint32_t i2 = indices[t*3+2];

        const float *p0 = verts[i0].position;
        const float *p1 = verts[i1].position;
        const float *p2 = verts[i2].position;
        const float *u0 = verts[i0].texcoord;
        const float *u1 = verts[i1].texcoord;
        const float *u2 = verts[i2].texcoord;

        float e1[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
        float e2[3] = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]};
        float du1 = u1[0]-u0[0], dv1 = u1[1]-u0[1];
        float du2 = u2[0]-u0[0], dv2 = u2[1]-u0[1];

        float det = du1*dv2 - du2*dv1;
        float r   = (fabsf(det) > 1e-8f) ? (1.0f / det) : 0.0f;

        float tx = (dv2*e1[0] - dv1*e2[0]) * r;
        float ty = (dv2*e1[1] - dv1*e2[1]) * r;
        float tz = (dv2*e1[2] - dv1*e2[2]) * r;
        float bx = (du1*e2[0] - du2*e1[0]) * r;
        float by = (du1*e2[1] - du2*e1[1]) * r;
        float bz = (du1*e2[2] - du2*e1[2]) * r;

        for (uint32_t k = 0; k < 3; k++) {
            uint32_t vi = indices[t*3+k];
            tan1[vi*3+0] += tx; tan1[vi*3+1] += ty; tan1[vi*3+2] += tz;
            tan2[vi*3+0] += bx; tan2[vi*3+1] += by; tan2[vi*3+2] += bz;
        }
    }

    /* Orthonormalise and write tangent + sign */
    for (uint32_t i = 0; i < vc; i++) {
        const float *n = verts[i].normal;
        float *t       = &tan1[i*3];
        float *b       = &tan2[i*3];

        /* Gram-Schmidt: T' = normalise(T - dot(T,N)*N) */
        float dot = n[0]*t[0] + n[1]*t[1] + n[2]*t[2];
        float tx  = t[0] - dot*n[0];
        float ty  = t[1] - dot*n[1];
        float tz  = t[2] - dot*n[2];
        float len = sqrtf(tx*tx + ty*ty + tz*tz);
        if (len < 1e-8f) { tx=1.f; ty=0.f; tz=0.f; } else { tx/=len; ty/=len; tz/=len; }

        /* Handedness: sign = dot(cross(N,T), B) */
        float cx = n[1]*tz - n[2]*ty;
        float cy = n[2]*tx - n[0]*tz;
        float cz = n[0]*ty - n[1]*tx;
        float sgn = (cx*b[0] + cy*b[1] + cz*b[2] < 0.0f) ? -1.0f : 1.0f;

        verts[i].tangent[0] = tx;
        verts[i].tangent[1] = ty;
        verts[i].tangent[2] = tz;
        verts[i].tangent[3] = sgn;
    }

    free(tan1);
    free(tan2);
}
