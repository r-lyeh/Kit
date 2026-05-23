#version 450
/*
 * mip_gen.comp
 *
 * Simple 2×2 box-filter mip downsampler for 2D RGBA8 textures.
 * Dispatched once per mip level: (dst_w/8, dst_h/8, 1).
 *
 * Push constant: src_mip = source mip level index.
 */

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D u_src;
layout(set = 1, binding = 0, rgba8) uniform writeonly image2D u_dst;

layout(push_constant) uniform PC { int src_mip; };

void main()
{
    ivec2 dst_coord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dst_size  = imageSize(u_dst);
    if (dst_coord.x >= dst_size.x || dst_coord.y >= dst_size.y) return;

    /* Sample 2×2 block from src mip using explicit LOD */
    vec2 src_size = vec2(textureSize(u_src, src_mip));
    vec2 uv       = (vec2(dst_coord) * 2.0 + 1.0) / (src_size);

    /* 2×2 gather using manual offsets */
    float dx = 0.5 / src_size.x;
    float dy = 0.5 / src_size.y;
    vec4 c  = textureLod(u_src, uv + vec2(-dx, -dy), float(src_mip));
    c      += textureLod(u_src, uv + vec2( dx, -dy), float(src_mip));
    c      += textureLod(u_src, uv + vec2(-dx,  dy), float(src_mip));
    c      += textureLod(u_src, uv + vec2( dx,  dy), float(src_mip));
    c      *= 0.25;

    imageStore(u_dst, dst_coord, c);
}
