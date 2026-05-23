#pragma once

extern struct mesh {
void     (*clear)(void);
void     (*vertex)(float2 pos, float4 col, float2 uv);
void     (*triangle)(int a, int b, int c);
void     (*quad)(int a, int b, int c, int d);
void     (*push)(unsigned texture);
array_(SDL_Vertex) vertices;
array_(int) indices;
} mesh;

void     mesh_clear(void) { array_clear(mesh.vertices); array_clear(mesh.indices); }
void     mesh_vertex(float2 pos, float4 col, float2 uv) { array_push(mesh.vertices, (SDL_Vertex){ {pos.x,pos.y},{col.x,col.y,col.z,col.w},{uv.x,uv.y} } ); }
void     mesh_triangle(int a, int b, int c) { array_push(mesh.indices, a); array_push(mesh.indices, b); array_push(mesh.indices, c); }
void     mesh_quad(int a, int b, int c, int d) { mesh.triangle(a,b,c); mesh.triangle(b,c,d); }
void     mesh_push(unsigned texture_id) { int cnt = array_count(mesh.vertices), idx = array_count(mesh.indices); if(cnt) SDL_RenderGeometry(render.handle, texture.handle(texture_id), mesh.vertices, cnt, idx ? mesh.indices : NULL, idx ); }
struct   mesh mesh = { .clear = mesh_clear, .vertex = mesh_vertex, .triangle = mesh_triangle, .push = mesh_push, .quad = mesh_quad };

