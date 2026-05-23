#pragma once

float2 tile_num(int at, int cols, int rows) { // select coords for a given tile in a [cols x rows] matrix
    int num = cols * rows;
    while( at < 0 ) at += num;
    while( at > num ) at -= num;
    return float2( at % cols, at / cols );
}
float4 tile_rect(int tile, float w, float h, float px) { // {x,y,w,h}
    float2 at = tile_num(tile, w / px, h / px );
    return float4( at.x * px, at.y * px, px, px );
}
float4 tile_uv(int tile, float w, float h, float px) { // {uv0,uv1}
    float4 r = tile_rect(tile, w, h, px);
    r.z += r.x; r.w += r.y; // x0,y0,x1,y1
    return float4( r.x / w, r.y / h, r.z / w, r.w / h );
}

void sprite_blit(unsigned tex, float2 pos, float2 dim, float2 anchor, float4 tint, float scale, float4 region) {
    dim.x *= scale; pos.x -= anchor.x;
    dim.y *= scale; pos.y -= anchor.y;
    mesh.clear();
    mesh.vertex(float2(pos.x      ,pos.y)      ,tint,float2(region.x,region.y));
    mesh.vertex(float2(pos.x+dim.x,pos.y)      ,tint,float2(region.z,region.y));
    mesh.vertex(float2(pos.x      ,pos.y+dim.y),tint,float2(region.x,region.w));
    mesh.vertex(float2(pos.x+dim.x,pos.y+dim.y),tint,float2(region.z,region.w));
    mesh.quad(0,1,2,3);
    mesh.push(tex);
}

void sprite(unsigned tex, float2 pos, float2 dim, float2 anchor, float4 tint, float scale) {
    sprite_blit(tex, pos, dim, anchor, tint, scale, float4(0,0,1,1));
}

