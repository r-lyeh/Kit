        /* Draw a single triangle with a different color at each vertex. Center this one and make it grow and shrink. */
        /* You always draw triangles with this, but you can string triangles together to form polygons. */
        mesh.clear();
        mesh.vertex( float2((WINDOW_WIDTH+0000)/2,(WINDOW_HEIGHT-size)/2), float4(1,0,0,1), float2(0,0));
        mesh.vertex( float2((WINDOW_WIDTH+size)/2,(WINDOW_HEIGHT+size)/2), float4(0,1,0,1), float2(0,0));
        mesh.vertex( float2((WINDOW_WIDTH-size)/2,(WINDOW_HEIGHT+size)/2), float4(0,0,1,1), float2(0,0));
        mesh.push(0);

        /* you can also map a texture to the geometry! Texture coordinates go from 0.0f to 1.0f. That will be the location
           in the texture bound to this vertex. */
        {
            mesh.clear();
            mesh.vertex(float2( 10, 10),float4(1,1,1,1),float2(0,0));
            mesh.vertex(float2(150, 10),float4(1,1,1,1),float2(1,0));
            mesh.vertex(float2( 10,150),float4(1,1,1,1),float2(0,1));
            mesh.push(tex);

            /* Did that only draw half of the texture? You can do multiple triangles sharing some vertices,
               using indices, to get the whole thing on the screen: */

            /* Let's just move this over so it doesn't overlap... */
            for (int i = 0; i < 3; i++) {
                mesh.vertices[i].position.x += 450;
            }

            /* we need one more vertex, since the two triangles can share two of them. */
            mesh.vertex(float2(600,150),float4(1,1,1,1),float2(1,1));

            /* And an index to tell it to reuse some of the vertices between triangles... */
            /* 4 vertices, but 6 actual places they used. Indices need less bandwidth to transfer and can reorder vertices easily! */
            mesh.triangle(0,1,2);
            mesh.triangle(1,2,3);
            mesh.push(tex);
        }
